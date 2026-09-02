#pragma once
#include <backend/font_manager/font_manager.hpp>

#include <frontend/fonts/font_awesome.hpp>
#include <frontend/fonts/jbm_reg.h>
#include <frontend/fonts/inter.hpp>
#include <frontend/fonts/inter_med.hpp>

// The single place to add, remove, or resize fonts.
// DPI scaling is automatic (ImGui's dynamic font atlas + style.FontScaleDpi)
// sizes below are base/design pixel sizes at 100% display scale.
inline const std::vector<FontData>& font_registry( ) {
    static const std::vector<FontData> fonts = {
        { "jbm_reg", (void*)jbm_reg, (int)jbm_reg_len, 16.0f, false, true },
        { "font_awesome", (void*)font_awesome, (int)font_awesome_len, 16.0f, true, false },
        { "jbm_reg_xl", (void*)jbm_reg, (int)jbm_reg_len, 18.0f, false, false },
        { "inter", (void*)inter, (int)inter_len, 18.0f, false, false },
        { "inter_medium", (void*)inter_med, (int)inter_med_len, 18.0f, false, false },
    };
    return fonts;
}
