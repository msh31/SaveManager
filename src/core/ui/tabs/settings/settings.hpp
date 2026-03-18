#pragma once
#include "core/ui/notifications/notification.hpp"
#include "core/network/network.hpp"
#include "core/globals.hpp"

#include "imgui/misc/cpp/imgui_stdlib.h"

namespace SettingsTab {
void render(const Fonts& fonts, Config& config);

inline constexpr std::string_view ubi_translation_url = "https://raw.githubusercontent.com/msh31/smdata/refs/heads/main/ubi_translations.json";
inline constexpr std::string_view steam_translation_url = "https://raw.githubusercontent.com/msh31/smdata/refs/heads/main/steamids.json";
};
