// clang-format off
#include "wine.hpp"
#include "../ubi/ubi.hpp"
#include "../rsg/rsg.hpp"
#include "../unreal/unreal.hpp"
#include "../ea/ea.hpp"
// clang-format on

std::string_view CWinePrefixDetector::name( ) const { return "Wine"; }

std::expected<std::vector<Game>, SMError> CWinePrefixDetector::find( ) {
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
            auto ubi_l_paths = CUbisoftDetector::scan( launcher_path, m_translations );
            std::ranges::move( ubi_l_paths, std::back_inserter( games ) );

            for ( const auto& user : fs::directory_iterator( users_dir ) ) {
                if ( user.path( ).filename( ) == "Public" ) continue;

                // ubisoft
                auto ubi_documents = CUbisoftDetector::scan( user.path( ) / "Documents", m_translations );
                auto ubi_anno_alt = CUbisoftDetector::scan( user.path( ) / "AppData" / "Roaming", m_translations );
                std::ranges::move( ubi_documents, std::back_inserter( games ) );
                std::ranges::move( ubi_anno_alt, std::back_inserter( games ) );

                // rockstar
                auto rsg_documents =
                    CRockstarDetector::scan( user.path( ) / "Documents" / "Rockstar Games", m_translations );
                auto rsg_leg_documents = CRockstarDetector::scan( user.path( ) / "Documents", m_translations );
                auto rsg_leg_appdata =
                    CRockstarDetector::scan( user.path( ) / "AppData" / "Local" / "Rockstar Games", m_translations );
                std::ranges::move( rsg_documents, std::back_inserter( games ) );
                std::ranges::move( rsg_leg_documents, std::back_inserter( games ) );
                std::ranges::move( rsg_leg_appdata, std::back_inserter( games ) );

                // unreal - TODO: expand beyond this hardcoded path.
                auto unreal = CUnrealDetector::scan_recursive( user.path( ), m_manifest_cache, m_name_cache );
                std::ranges::move( unreal, std::back_inserter( games ) );

                // ea
                auto ea_documents = CElectronicArtsDetector::scan( user.path( ) / "Documents", m_translations );
                auto ea_appdata = CElectronicArtsDetector::scan( user.path( ) / "AppData", m_translations );
                auto ea_programdata =
                    CElectronicArtsDetector::scan( user.path( ) / "ProgramData" / "Electronic Arts", m_translations );
                std::ranges::move( ea_documents, std::back_inserter( games ) );
                std::ranges::move( ea_appdata, std::back_inserter( games ) );
                std::ranges::move( ea_programdata, std::back_inserter( games ) );
            }
        } catch ( const fs::filesystem_error& fse ) {
            // TODO: do something with this, logging is spammy on linux
            //  SPDLOG_WARN( "filesystem error occured, skipping {}: {}", entry.path( ).string( ), fse.what( ) );
        }
    }

    return games;
}
