#pragma once
#include <utils/utils.hpp>

class Config;

struct SettingsTab {
    void render(const Fonts& fonts, Config& config);
    // bool* m_refresh_requested = nullptr;

    std::string_view ubi_translation_url = "https://raw.githubusercontent.com/msh31/SaveManager/refs/heads/dev/data/ubi_translations.json";
    std::string_view steam_translation_url = "https://raw.githubusercontent.com/msh31/SaveManager/refs/heads/dev/data/steamids.json";

    std::string blacklist_input;
    std::string new_game_name;
    std::string new_game_path; 
    std::string new_game_appid;

    std::future<bool> update_future;
    std::future<std::pair<bool, bool>> update_t_future;

    int spinner_frame = 0;
    const char* spinner = "|/-\\";
    const Fonts* m_fonts = nullptr;
};
