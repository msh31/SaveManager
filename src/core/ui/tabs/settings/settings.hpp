#pragma once
#include "core/globals.hpp"

class Config;

struct SettingsTab {
    void render(const Fonts& fonts, Config& config);

    std::string_view ubi_translation_url = "https://raw.githubusercontent.com/msh31/smdata/refs/heads/main/ubi_translations.json";
    std::string_view steam_translation_url = "https://raw.githubusercontent.com/msh31/smdata/refs/heads/main/steamids.json";

    std::string blacklist_input;
    std::string new_game_name;
    std::string new_game_path; 
    std::string new_game_appid;

    std::future<bool> update_future;

    std::string backup_path;
    std::string steam_path;
    std::string lutris_path;
    std::string heroic_path;
    bool paths_initialized = false;
};
