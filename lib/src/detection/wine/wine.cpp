// clang-format off
#include <detection/wine/wine.hpp>
#include <detection/ubi/ubi.hpp>
#include <detection/rsg/rsg.hpp>
#include <detection/unreal/unreal.hpp>
// clang-format on

CWinePrefixDetector::CWinePrefixDetector( fs::path prefix ) : m_path( std::move( prefix ) ) {}

std::string_view CWinePrefixDetector::name( ) const { return "Wine"; }

std::expected<std::vector<Game>, SMError> CWinePrefixDetector::find( ) {
#ifdef _WIN32
    return { }; // TODO: return warning
#endif

    if ( !fs::exists( m_path ) ) return { };
    std::vector<Game> games = { };

    for ( const auto& entry : fs::directory_iterator( m_path ) ) {
        try {
            fs::path prefix = entry.path( );
            if ( !fs::exists( prefix ) ) continue;

            fs::path drive_c = fs::exists( prefix / "pfx" ) ? prefix / "pfx/drive_c" : prefix / "drive_c";
            fs::path users_dir = drive_c / "users";

            if ( !fs::exists( users_dir ) ) continue;

            // ubisoft's special case
            fs::path launcher_path =
                drive_c / "Program Files (x86)" / "Ubisoft" / "Ubisoft Game Launcher" / "savegames";
            auto ubi_l_paths = CUbisoftDetector::scan( launcher_path );
            std::ranges::move( ubi_l_paths, std::back_inserter( games ) );

            for ( const auto& user : fs::directory_iterator( users_dir ) ) {
                if ( user.path( ).filename( ) == "Public" ) continue;

                // ubisoft
                auto ubi_documents = CUbisoftDetector::scan( user.path( ) / "Documents" );
                auto ubi_anno = CUbisoftDetector::scan( user.path( ) / "Documents" );
                auto ubi_anno_alt = CUbisoftDetector::scan( user.path( ) / "AppData" / "Roaming" );
                std::ranges::move( ubi_documents, std::back_inserter( games ) );
                std::ranges::move( ubi_anno, std::back_inserter( games ) );
                std::ranges::move( ubi_anno_alt, std::back_inserter( games ) );

                // rockstar
                auto rsg_documents = CRockstarDetector::scan( user.path( ) / "Documents" / "Rockstar Games" );
                auto rsg_leg_documents = CRockstarDetector::scan( user.path( ) / "Documents" );
                auto rsg_leg_appdata = CRockstarDetector::scan( user.path( ) / "AppData" / "Local" / "Rockstar Games" );
                std::ranges::move( rsg_documents, std::back_inserter( games ) );
                std::ranges::move( rsg_leg_documents, std::back_inserter( games ) );
                std::ranges::move( rsg_leg_appdata, std::back_inserter( games ) );

                // unreal - TODO: expand beyond this hardcoded path.
                auto unreal = CUnrealDetector::scan_recursive( user.path( ) );
                std::ranges::move( unreal, std::back_inserter( games ) );
            }
        } catch ( const fs::filesystem_error& fse ) {
            SPDLOG_WARN( "filesystem error occured, skipping {}: {}", entry.path( ).string( ), fse.what( ) );
        }
    }

    return games;
}
