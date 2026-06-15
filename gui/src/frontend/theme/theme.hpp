#pragma once

enum class ThemeType { Dark, Light, Custom, Default = Dark };

namespace ThemeManager {
    void apply_colors( ThemeType theme );
    void apply_style( );
    inline ThemeType currentTheme;

    inline ImGuiStyle style;
}; // namespace ThemeManager
