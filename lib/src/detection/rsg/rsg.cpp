#include "detection/rsg/rsg.hpp"
#include "utils/translations/translations.hpp"

std::expected<std::vector<Game>, SMError> CRockstarDetector::find( ) {
    std::vector<fs::path> prefixes = { };
#ifdef _WIN32
    prefixes.emplace_back( paths::documents_dir( ) / "Rockstar Games" );
    prefixes.emplace_back( paths::documents_dir( ) );                                     // legacy
    prefixes.emplace_back( paths::home_dir( ) / "AppData" / "Local" / "Rockstar Games" ); // legacy
#endif

    std::vector<Game> games = { };
    for ( const auto& prefix : prefixes ) {
        if ( !fs::exists( prefix ) ) continue;
        auto found_games = scan( prefix );
        std::ranges::move( found_games, std::back_inserter( games ) );
    }
    return games;
}

std::vector<Game> CRockstarDetector::scan( fs::path path ) {
    if ( !fs::exists( path ) ) return { };
    std::vector<Game> games = { };

    const std::unordered_map<std::string_view, std::string> legacy_games = {
        { "GTA3 User Files", "Grand Theft Auto III" },
        { "GTA Vice City User Files", "Grand Theft Auto Vice City" },
        { "GTA San Andreas User Files", "Grand Theft Auto San Andreas" },
        { "Manhunt User Files", "Manhunt" },
        { "Manhunt 2", "Manhunt 2" },
    };

    for ( const auto& game : fs::directory_iterator( path, fs::directory_options::skip_permission_denied ) ) {
        fs::path    game_folder     = game.path( );
        std::string folder_name     = game_folder.filename( ).string( );
        fs::path    profiles_folder = game_folder / "Profiles";

        // legacy game search
        if ( auto it = legacy_games.find( folder_name ); it != legacy_games.end( ) ) {
            auto& [key, game_name] = *it;
            Game l_game;
            l_game.type      = PlatformType::ROCKSTAR;
            l_game.game_name = game_name;
            l_game.appid     = translations::get_steam_id( game_name ).value_or( "N/A" );
            l_game.save_paths.push_back( game.path( ) );

            games.push_back( l_game );
        }

        // modern game search
        if ( folder_name == "Launcher" || folder_name == "Social Club" ) {
            continue;
        }

        if ( !fs::exists( profiles_folder ) ) {
            continue;
        }

        for ( const auto& profile : fs::directory_iterator( profiles_folder ) ) {
            fs::path uuid_folder = profile.path( );

            Game game;
            game.type      = PlatformType::ROCKSTAR;
            game.game_name = translations::get_game_name_rsg( folder_name ).value_or( folder_name );
            game.appid     = translations::get_steam_id( game.game_name ).value_or( "N/A" );
            game.save_paths.push_back( uuid_folder );

            games.push_back( game );
        }
    }

    return games;
}

std::string_view CRockstarDetector::name( ) const { return "Rockstar"; }
