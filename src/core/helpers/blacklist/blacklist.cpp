#include "blacklist.hpp"
#include "core/helpers/paths.hpp"
#include "core/logger/logger.hpp"

void Blacklist::init() {
    json data;
    std::ifstream file(paths::blacklist().string().c_str());

    if (file.is_open()) {
        data = json::parse(file);
        get_logger().info("Loaded blacklist JSON");

        for (const auto& entry : data) {
            blacklisted_games.insert(entry.get<std::string>());
        }
    } else {
        get_logger().error("Failed to open blacklist to load it!");
    }
}

void Blacklist::save() {
    json data;
    for (const auto& entry : blacklisted_games) {
        data.emplace_back(entry);
    }

    std::ofstream file(paths::blacklist().string().c_str());
    file << data.dump(4);
}

bool Blacklist::is_blacklisted(const std::string& game_name) {
    return blacklisted_games.count(game_name) > 0;
}
