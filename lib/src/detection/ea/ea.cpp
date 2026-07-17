#include "ea.hpp"
#include "utils/translations/translations.hpp"

std::expected<std::vector<Game>, SMError> CElectronicArtsDetector::find( ) {
    std::vector<fs::path> prefixes = { };
#ifdef _WIN32
    prefixes.emplace_back( paths::documents_dir( ) );                                // NFS
    prefixes.emplace_back( paths::home_dir( ) / "AppData" );                         // Battlefield
    prefixes.emplace_back( paths::home_dir( ) / "ProgramData" / "Electronic Arts" ); // Legacy
#endif

    std::vector<Game> games = { };
    for ( const auto& prefix : prefixes ) {
        if ( !fs::exists( prefix ) ) continue;
        SPDLOG_INFO( "[EA] searching prefix: {}", prefix.string( ) );

        auto found_games = scan( prefix, m_translations );
        std::ranges::move( found_games, std::back_inserter( games ) );
    }
    return games;
}

std::vector<Game> CElectronicArtsDetector::scan( fs::path path, const Translations& translations ) {
    if ( !fs::exists( path ) ) return { };
    std::vector<Game> games = { };

    for ( const auto& game : fs::directory_iterator( path, fs::directory_options::skip_permission_denied ) ) {
        fs::path game_folder = game.path( );
        std::string folder_name = game_folder.filename( ).string( );
        fs::path profiles_folder = game_folder / "Profiles";

        for ( const auto& profile : fs::directory_iterator( profiles_folder ) ) {
            fs::path uuid_folder = profile.path( );

            Game game;
            game.type = PlatformType::EA;
            // game.game_name = translations.get_game_name_rsg( folder_name ).value_or( folder_name );
            // game.appid = translations.get_steam_id( game.game_name ).value_or( "N/A" );
            game.save_paths.push_back( uuid_folder );

            SPDLOG_INFO( "[EA] found: {}", game.game_name );
            games.push_back( game );
        }
    }

    return games;
}

std::string_view CElectronicArtsDetector::name( ) const { return "EA"; }
