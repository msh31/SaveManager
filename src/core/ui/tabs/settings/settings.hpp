#pragma once
#include "core/globals.hpp"

class Config;

namespace SettingsTab {
void render(const Fonts& fonts, Config& config);

inline constexpr std::string_view ubi_translation_url = "https://raw.githubusercontent.com/msh31/smdata/refs/heads/main/ubi_translations.json";
inline constexpr std::string_view steam_translation_url = "https://raw.githubusercontent.com/msh31/smdata/refs/heads/main/steamids.json";
};
