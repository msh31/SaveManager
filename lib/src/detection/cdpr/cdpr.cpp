#include "cdpr.hpp"
#include "../../utils/save_helper/save_helper.hpp"

namespace {
    const std::vector<SaveLocation>& table( ) {
        // TODO: add TW1/2, wtf else did this company make
        static const std::vector<SaveLocation> table = {
            { "Cyberpunk 2077", SaveRoot::SAVED_GAMES, "CD Projekt Red/Cyberpunk 2077", std::nullopt, true },
            { "The Witcher 3", SaveRoot::DOCUMENTS, "The Witcher 3/gamesaves", std::nullopt, true },
        };
        return table;
    }

    Game make_game( std::string name, fs::path save_path ) {
        Game game;
        game.type = PlatformType::CDPROJEKTRED;
        game.platform_label = std::string( CCDPRDetector::PLATFORM_LABEL );
        game.game_name = std::move( name );
        game.appid = "N/A";
        game.save_paths.push_back( std::move( save_path ) );
        game.show_parent_path = true;
        return game;
    }
} // namespace

std::string_view CCDPRDetector::name( ) const { return PLATFORM_LABEL; }

std::expected<std::vector<Game>, SMError> CCDPRDetector::find( ) {
    std::unordered_map<SaveRoot, fs::path> roots;
#ifdef _WIN32
    roots[SaveRoot::DOCUMENTS] = save::resolve_root( SaveRoot::DOCUMENTS );
    roots[SaveRoot::SAVED_GAMES] = save::resolve_root( SaveRoot::SAVED_GAMES );
#endif

    auto games = save::scan_locations( roots, table( ), PlatformType::CDPROJEKTRED, PLATFORM_LABEL );
    return games;
}

std::vector<Game> CCDPRDetector::scan_wine_user( const fs::path& user_home, const DetectorContext& ctx ) {
    std::unordered_map<SaveRoot, fs::path> roots = {
        { SaveRoot::DOCUMENTS, user_home / "Documents" },
        { SaveRoot::SAVED_GAMES, user_home / "Saved Games" },
    };

    auto games = save::scan_locations( roots, table( ), PlatformType::CDPROJEKTRED, PLATFORM_LABEL );
    return games;
}
