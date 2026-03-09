#pragma once
#include <string>
#include <fstream>
#include <optional>
#include <regex>

#include "../../external/json.hpp"
#include "core/helpers/paths.hpp"
#include "core/logger/logger.hpp"

using json = nlohmann::json;

inline std::optional<std::string> get_game_name_ubi(const std::string& game_id) {
    static json data;
    static bool loaded = false;
    
    if (!loaded) {
        std::ifstream file(ubi_translations.c_str());
        if (!file.is_open()) {
            get_logger().error("Failed to open ubi translations file!");
            return std::nullopt;
        }
        data = json::parse(file);
        get_logger().info("Loaded ubi_translations JSON with " + std::to_string(data.size()) + " franchises");
        loaded = true;
    }
    
    for (const auto& [franchise, games] : data.items()) {
        // std::cout << "Searching franchise: " << franchise << "\n";
        if (games.contains(game_id)) {
            auto name = std::regex_replace(games[game_id].get<std::string>(), std::regex(R"(\s*\([^)]*\))"), "");
            return name; 
        }
    }
    
    return std::nullopt;
}

inline std::optional<std::string> get_game_name_rsg(const std::string& game_id) {
    static json data;
    static bool loaded = false;
    
    if (!loaded) {
        std::ifstream file(rsg_translations.c_str());
        if (!file.is_open()) {
            get_logger().error("Failed to open rsg translations file!");
            return std::nullopt;
        }
        data = json::parse(file);
        get_logger().info("Loaded rsg_translations JSON with " + std::to_string(data.size()) + " entries");
        loaded = true;
    }
    
    if (data.contains(game_id)) {
        return data[game_id].get<std::string>();
    }
    return std::nullopt;
}

inline std::optional<std::string> get_steam_id(const std::string& game_name) {
    static json data;
    static bool loaded = false;
    
    if (!loaded) {
        fs::path json_path = config_dir / "steamids.json";
        // std::string json_utf8 = json_path.u8string();
        std::ifstream file(json_path.c_str());
        if (!file.is_open()) {
            get_logger().error("Failed to open steamids file!");
            return std::nullopt;
        }
        data = json::parse(file);
        get_logger().info("Loaded steamids.json!");
        loaded = true;
    }
    
    for (const auto& [platform, games] : data.items()) {
        for (const auto& game : games) {
            if (game["name"] == game_name) {
                return std::to_string(game["appid"].get<int>());
            }
        }
    }
    
    return std::nullopt;
}
