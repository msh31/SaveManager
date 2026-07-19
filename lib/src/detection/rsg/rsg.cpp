#include "rsg.hpp"
#include "../detector_utils.hpp"
#include "utils/translations/translations.hpp"

std::expected<std::vector<Game>, SMError> CRockstarDetector::find( ) {
    std::vector<fs::path> prefixes = { };
#ifdef _WIN32
    prefixes.emplace_back( paths::documents_dir( ) / "Rockstar Games" );
    prefixes.emplace_back( paths::documents_dir( ) );                                     // legacy
    prefixes.emplace_back( paths::home_dir( ) / "AppData" / "Local" / "Rockstar Games" ); // legacy
#endif

    return scan_prefixes(
        PLATFORM_LABEL, prefixes, [this]( const fs::path& p ) { return scan( p, m_translations ); } );
}

std::vector<Game> CRockstarDetector::scan_wine_user( const fs::path& user_home, const DetectorContext& ctx ) {
    std::vector<Game> games;
    auto documents = scan( user_home / "Documents" / "Rockstar Games", ctx.translations );
    auto legacy_documents = scan( user_home / "Documents", ctx.translations );
    auto legacy_appdata = scan( user_home / "AppData" / "Local" / "Rockstar Games", ctx.translations );
    std::ranges::move( documents, std::back_inserter( games ) );
    std::ranges::move( legacy_documents, std::back_inserter( games ) );
    std::ranges::move( legacy_appdata, std::back_inserter( games ) );
    return games;
}

std::vector<Game> CRockstarDetector::scan( fs::path path, const Translations& translations ) {
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
        fs::path game_folder = game.path( );
        std::string folder_name = game_folder.filename( ).string( );
        fs::path profiles_folder = game_folder / "Profiles";

        // legacy game search
        if ( auto it = legacy_games.find( folder_name ); it != legacy_games.end( ) ) {
            auto& [key, game_name] = *it;
            Game l_game;
            l_game.type = PlatformType::ROCKSTAR;
            l_game.platform_label = std::string( PLATFORM_LABEL );
            l_game.game_name = game_name;
            l_game.save_paths.push_back( game.path( ) );

            SPDLOG_INFO( "[Rockstar] found legacy title: {}", game_name );
            games.push_back( l_game );
        }

        // modern game search
        if ( folder_name == "Launcher" || folder_name == "Social Club" ) {
            continue;
        }

        if ( !fs::exists( profiles_folder ) ) {
            /*SPDLOG_WARN( "[Rockstar] unable to find any profiles: {}", profiles_folder.string() );*/
            continue;
        }

        for ( const auto& profile : fs::directory_iterator( profiles_folder ) ) {
            fs::path uuid_folder = profile.path( );

            Game game;
            game.type = PlatformType::ROCKSTAR;
            game.platform_label = std::string( PLATFORM_LABEL );
            game.game_name = translations.get_game_name_rsg( folder_name ).value_or( folder_name );
            game.save_paths.push_back( uuid_folder );

            SPDLOG_INFO( "[Rockstar] found: {}", game.game_name );
            games.push_back( game );
        }
    }

    return games;
}

std::string_view CRockstarDetector::name( ) const { return PLATFORM_LABEL; }
