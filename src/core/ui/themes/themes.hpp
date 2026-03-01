#pragma once
#include "imgui.h"

enum class ThemeType {
    Dark,
    Light,
    Custom,
    Default = Dark
};

class ThemeManager {
    public:
        static void apply_theme(ThemeType theme);
        static ThemeType get_current_theme() { return currentTheme; }
        
    private:
        static ThemeType currentTheme;
};
