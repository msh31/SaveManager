#include "custom_games.hpp"
#include "core/helpers/paths.hpp"
#include "core/logger/logger.hpp"

void CustomGamesFile::init() {
    json data;
    std::ifstream file(paths::custom_games().string().c_str());

    if (file.is_open()) {
        data = json::parse(file);
        get_logger().info("Loaded custom detections json!");

        for (const auto& entry : data) {
            CustomGame g;
            g.game_name = entry.value("game_name", std::string(""));
            g.save_path = entry.value("save_path", std::string(""));
            g.appid = entry.value("appid", std::string("N/A"));
            games.push_back(g);
        }
    } else {
        get_logger().error("Failed to open custom detections to load it!");
    }
}

void CustomGamesFile::save() {
    json data;
    for (const auto& entry : games) {
        json obj;
        obj["game_name"] = entry.game_name;
        obj["save_path"] = entry.save_path;
        obj["appid"] = entry.appid;
        data.push_back(obj);
    }

    std::ofstream file(paths::custom_games().string().c_str());
    file << data.dump(4);
}
