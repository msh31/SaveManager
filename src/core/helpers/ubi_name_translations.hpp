#pragma once
#include <string>
#include <fstream>
#include <optional>
#include <iostream>

#include "../../external/json.hpp"
#include "core/helpers/paths.hpp"

using json = nlohmann::json;

inline std::optional<std::string> getGameName(const std::string& game_id) {
    static json data;
    static bool loaded = false;
    
    if (!loaded) {
        fs::path json_path = config_dir / "gameids.json";
        // std::string json_utf8 = json_path.u8string();
        std::ifstream file(json_path.c_str());
        if (!file.is_open()) {
            std::cerr << "Failed to open JSON file\n";
            return std::nullopt;
        }
        data = json::parse(file);
        std::cout << "Loaded JSON with " << data.size() << " franchises\n";
        loaded = true;
    }
    
    for (const auto& [franchise, games] : data.items()) {
        // std::cout << "Searching franchise: " << franchise << "\n";
        if (games.contains(game_id)) {
            return games[game_id].get<std::string>();
        }
    }
    
    return std::nullopt;
}
