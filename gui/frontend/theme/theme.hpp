#pragma once

enum class ThemeType { Dark, Light, Custom, Default = Dark };

namespace ThemeManager {
    void apply_colors( ThemeType theme );
    void apply_style( );
    void apply_scale( float scale ); // rescales apply_style()'s base sizes, e.g. on DPI change

    inline ThemeType current_theme;
    inline bool has_applied = false;

    inline ImGuiStyle style;
}; // namespace ThemeManager
