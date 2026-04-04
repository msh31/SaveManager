#include "translations.hpp"
#include "backend/utils/paths.hpp"
#include "backend/logger/logger.hpp"

static std::unordered_map<std::string, std::string> ubi_translations;
static std::unordered_map<std::string, std::string> steam_n2i_translation; //name to appid
static std::unordered_map<std::string, std::string> steam_i2n_translation; //appid to name
static const std::unordered_map<std::string_view, std::string> rsg_translations = { //small enough, for now.
    {"GTAV", "Grand Theft Auto V"},
    {"GTAV Enhanced", "Grand Theft Auto V Enhanced"},
    {"GTA San Andreas Definitive Edition", "Grand Theft Auto: San Andreas \u2013 The Definitive Edition"},
    {"GTA Vice City Definitive Edition", "Grand Theft Auto: Vice City \u2013 The Definitive Edition"},
    {"GTA III Definitive Edition", "Grand Theft Auto III \u2013 The Definitive Edition"},
    {"GTA San Andreas", "Grand Theft Auto: San Andreas"},
    {"GTA Vice City", "Grand Theft Auto: Vice City"},
    {"GTA III", "Grand Theft Auto III"},
    {"Bully", "Bully: Scholarship Edition"},
};

void translations::init() {
    json ubi_data, steam_data;
    std::ifstream ubi_file(paths::ubi_translations().c_str());
    std::ifstream steam_file(paths::steam_appids().c_str());

    try {
        if (ubi_file.is_open() && fs::file_size(paths::ubi_translations()) > 0) {
            ubi_data = json::parse(ubi_file);
            get_logger().info("Loaded ubi_translations JSON with " + std::to_string(ubi_data.size()) + " franchises");
            for (const auto& [franchise, games] : ubi_data.items()) {
                for (const auto& [id, name] : games.items()) {
                    auto clean = std::regex_replace(name.get<std::string>(), std::regex(R"(\s*\([^)]*\))"), "");
                    ubi_translations[id] = clean;
                }
            }
        } else {
            get_logger().error("Failed to open ubi translations file!");
        }
    } catch(json::exception& ex) {
        get_logger().error("Translation error: " + std::string(ex.what()));
    }

    try {
        if (steam_file.is_open() && fs::file_size(paths::steam_appids()) > 0) {
            steam_data = json::parse(steam_file);
            get_logger().info("Loaded steamids.json!");
            for (const auto& [platform, games] : steam_data.items()) {
                for (const auto& game : games) {
                    std::string name = game["name"].get<std::string>();
                    std::string appid = std::to_string(game["appid"].get<int>());
                    steam_n2i_translation[name] = appid;
                    steam_i2n_translation[appid] = name;
                }
            } 
        } else {
            get_logger().error("Failed to open steamids file!");
        }
    } catch(json::exception& ex) {
        get_logger().error("Translation error: " + std::string(ex.what()));
    }
}

std::optional<std::string> translations::get_game_name_ubi(const std::string& game_id) {
    if (auto it = ubi_translations.find(game_id); it != ubi_translations.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::optional<std::string> translations::get_steam_id(const std::string& game_name) {    
    if (auto it = steam_n2i_translation.find(game_name); it != steam_n2i_translation.end()) {
        return it->second;
    }
    
    return std::nullopt;
}


std::optional<std::string> translations::get_steam_name(const std::string& appid) {
    if (auto it = steam_i2n_translation.find(appid); it != steam_i2n_translation.end()) {
        return it->second;
    }
    
    return std::nullopt;
}

std::optional<std::string> translations::get_game_name_rsg(std::string_view folder_name) {
    if (auto it = rsg_translations.find(folder_name); it != rsg_translations.end()) {
        return it->second;
    }
    return std::nullopt;
}

