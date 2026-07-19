#include "ea.hpp"
#include "../detector_utils.hpp"
#include "utils/translations/translations.hpp"

std::expected<std::vector<Game>, SMError> CElectronicArtsDetector::find( ) {
    std::vector<fs::path> prefixes = { };
#ifdef _WIN32
    prefixes.emplace_back( paths::documents_dir( ) ); // NFS
    // prefixes.emplace_back( paths::home_dir( ) / "AppData" );                         // Battlefield
    // prefixes.emplace_back( paths::home_dir( ) / "ProgramData" / "Electronic Arts" ); // Legacy
#endif

    return scan_prefixes( PLATFORM_LABEL, prefixes, [this]( const fs::path& p ) { return scan( p, m_translations ); } );
}

std::vector<Game> CElectronicArtsDetector::scan_wine_user( const fs::path& user_home, const DetectorContext& ctx ) {
    std::vector<Game> games;
    auto documents = scan( user_home / "Documents", ctx.translations );
    auto appdata = scan( user_home / "AppData", ctx.translations );
    std::ranges::move( documents, std::back_inserter( games ) );
    std::ranges::move( appdata, std::back_inserter( games ) );
    return games;
}

std::vector<Game> CElectronicArtsDetector::scan_wine_prefix( const fs::path& drive_c, const DetectorContext& ctx ) {
    return scan( drive_c / "ProgramData" / "Electronic Arts", ctx.translations );
}

std::vector<Game> CElectronicArtsDetector::scan( fs::path path, const Translations& translations ) {
    static char NFS_HEADER[4] = { 'N', 'F', 'S', 'S' };
    if ( !fs::exists( path ) ) return { };
    std::vector<Game> games = { };

    for ( const auto& game : fs::directory_iterator( path, fs::directory_options::skip_permission_denied ) ) {
        fs::path game_folder = game.path( );
        std::string folder_name = game_folder.filename( ).string( );

        // This catches the following:
        // NFS Unbound, Heat and Payback
        // if ( folder_name == "Criterion Games" || folder_name == "Ghost Games" ) {
        // if ( game_folder.filename( ).extension( ) != ".sav" ) continue;
        // if ( !fs::exists( folder_name ) ) continue;

        // std::ifstream save( game_folder, std::ifstream::binary );
        // if ( !save.is_open( ) ) continue;

        // char buffer[4];
        // save.read( buffer, 4 );
        // if ( save.gcount( ) != 4 ) continue;

        // if ( !std::ranges::equal( buffer, NFS_HEADER ) ) {
        //     if ( save.is_open( ) ) save.close( );
        //     continue;
        // }
        //}

        SPDLOG_INFO( "[EA] found: {}", folder_name );

        // Game game;
        // game.type = PlatformType::EA;
        //// game.game_name = translations.get_game_name_rsg( folder_name ).value_or( folder_name );
        //// game.appid = translations.get_steam_id( game.game_name ).value_or( "N/A" );
        // game.save_paths.push_back( uuid_folder );

        // games.push_back( game );
    }

    return games;
}

std::string_view CElectronicArtsDetector::name( ) const { return PLATFORM_LABEL; }
