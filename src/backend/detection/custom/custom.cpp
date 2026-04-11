#include "custom.hpp"

std::expected<std::vector<Game>, DetectionError> CustomDetector::find_saves(const fs::path& prefix) const {
    if(!fs::exists(prefix)) {
        return std::unexpected{DetectionError::PathNotFound};
    }
    std::vector<Game> games;

    auto process_entry = [&](const auto& entry) {
        fs::path save_path = prefix / entry.save_path;
        if (!fs::exists(save_path)) {
            return;
        }

        Game game;
        game.type = PlatformType::CUSTOM;
        game.game_name = entry.game_name;
        game.appid = entry.appid;
        game.save_path = save_path;

        games.push_back(game);
    };

    for (const auto& entry : CustomGamesFile::games) process_entry(entry);
    for (const auto& entry : default_games) process_entry(entry);

    return games;
}
