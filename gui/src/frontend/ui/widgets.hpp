#pragma once
#include <utils/utils.hpp>

namespace Widgets {
constexpr float button_spacing = 4.0f;
constexpr float btn_width = 80.0f;
constexpr const char *spinner = "|/-\\";

bool begin_game_card( const char *id, const Fonts &fonts, bool &expanded, const char *title, const char *right_text );

void end_game_card( );
} // namespace Widgets
