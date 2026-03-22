#pragma once
#include "core/globals.hpp"

class Config;

struct SettingsTab {
    void render(const Fonts& fonts, Config& config);

    std::string_view ubi_translation_url = "https://raw.githubusercontent.com/msh31/smdata/refs/heads/main/ubi_translations.json";
    std::string_view steam_translation_url = "https://raw.githubusercontent.com/msh31/smdata/refs/heads/main/steamids.json";

    std::future<bool> update_future;
};
