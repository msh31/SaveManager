#include "custom.hpp"
#include <backend/logger/logger.hpp>

std::expected<std::vector<Game>, DetectionError> CustomDetector::find_saves(const fs::path& prefix) const {
    if(!fs::exists(prefix)) {
        return std::unexpected{DetectionError::PathNotFound};
    }
    std::vector<Game> games;

    //TODO: extract to a lambda
    for (const auto& entry : CustomGamesFile::games) {
        fs::path save_path = prefix / entry.save_path;
         //get_logger().info("Custom check: " + save_path.string());

        if (!fs::exists(save_path)) {
            continue;
        }

        Game game;
        game.type = PlatformType::CUSTOM;
        game.game_name = entry.game_name;
        game.appid = entry.appid;
        game.save_path = save_path;

        games.push_back(game);
    }
    for (const auto& entry : default_games) {
        fs::path save_path = prefix / entry.save_path;
        //get_logger().info("Custom check: " + save_path.string());

        if (!fs::exists(save_path)) {
            continue;
        }

        Game game;
        game.type = PlatformType::CUSTOM;
        game.game_name = entry.game_name;
        game.appid = entry.appid;
        game.save_path = save_path;

        games.push_back(game);
    }

    return games;
}
