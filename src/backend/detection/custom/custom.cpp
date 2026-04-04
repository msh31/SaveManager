#include "custom.hpp"
#include <backend/logger/logger.hpp>

void CustomDetector::find_saves(const fs::path& prefix, std::vector<Game>& out_games) const {
    if(!fs::exists(prefix)) {
        return;
    }

    //TODO: extract to a lambda
    for (const auto& entry : CustomGamesFile::games) {
        fs::path save_path = prefix / entry.save_path;
         //get_logger().info("Custom check: " + save_path.string());

        if (!fs::exists(save_path)) {
            continue;
        }

        Game game;
        game.type = CUSTOM;
        game.game_name = entry.game_name;
        game.appid = entry.appid;
        game.save_path = save_path;

        out_games.push_back(game);
    }
    for (const auto& entry : default_games) {
        fs::path save_path = prefix / entry.save_path;
        //get_logger().info("Custom check: " + save_path.string());

        if (!fs::exists(save_path)) {
            continue;
        }

        Game game;
        game.type = CUSTOM;
        game.game_name = entry.game_name;
        game.appid = entry.appid;
        game.save_path = save_path;

        out_games.push_back(game);
    }
}
