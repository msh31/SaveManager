#pragma once
#include <string>
#include <fstream>
#include <optional>
#include <regex>
#include <unordered_map>

#include "../../external/json.hpp"
#include "core/helpers/paths.hpp"
#include "core/logger/logger.hpp"

using json = nlohmann::json;

inline std::optional<std::string> get_game_name_ubi(const std::string& game_id) {
    static json data;
    static bool loaded = false;
    
    if (!loaded) {
        std::ifstream file(paths::ubi_translations().c_str());
        if (!file.is_open()) {
            get_logger().error("Failed to open ubi translations file!");
            return std::nullopt;
        }
        data = json::parse(file);
        get_logger().info("Loaded ubi_translations JSON with " + std::to_string(data.size()) + " franchises");
        loaded = true;
    }
    
    for (const auto& [franchise, games] : data.items()) {
        if (games.contains(game_id)) {
            auto name = std::regex_replace(games[game_id].get<std::string>(), std::regex(R"(\s*\([^)]*\))"), "");
            return name; 
        }
    }
    
    return std::nullopt;
}

inline std::optional<std::string> get_game_name_rsg(const std::string& folder_name) {
    static const std::unordered_map<std::string, std::string> translations = {
        {"GTAV", "Grand Theft Auto V"},
        {"GTAV Enhanced", "Grand Theft Auto V Enhanced"},
        {"GTA San Andreas Definitive Edition", "Grand Theft Auto: San Andreas \u2013 The Definitive Edition"},
        {"GTA Vice City Definitive Edition", "Grand Theft Auto: Vice City \u2013 The Definitive Edition"},
        {"GTA III Definitive Edition", "Grand Theft Auto III \u2013 The Definitive Edition"},
        {"GTA San Andreas", "Grand Theft Auto: San Andreas"},
        {"GTA Vice City", "Grand Theft Auto: Vice City"},
        {"GTA III", "Grand Theft Auto III"},
    };

    auto it = translations.find(folder_name);
    if (it != translations.end()) return it->second;
    return std::nullopt;
}

inline std::optional<std::string> get_steam_id(const std::string& game_name) {
    static json data;
    static bool loaded = false;
    
    if (!loaded) {
        std::ifstream file(paths::steam_appids().c_str());
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
