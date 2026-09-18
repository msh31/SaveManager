#pragma once
#include <backend/font_manager/font_manager.hpp>

#include <frontend/fonts/font_awesome.hpp>
#include <frontend/fonts/jbm_reg.h>
#include <frontend/fonts/jbm_bold.h>
#include <frontend/fonts/jbm_med.h>

// The single place to add, remove, or resize fonts.
// DPI scaling is automatic (ImGui's dynamic font atlas + style.FontScaleDpi)
// sizes below are base/design pixel sizes at 100% display scale.
inline const std::vector<FontData>& font_registry( ) {
    static const std::vector<FontData> fonts = {
        { "jbm_reg", (void*)jbm_reg, (int)jbm_reg_len, 16.0f, false, true },
        { "font_awesome", (void*)font_awesome, (int)font_awesome_len, 16.0f, true, false },
        { "jbm_reg_xl", (void*)jbm_reg, (int)jbm_reg_len, 18.0f, false, false },
        { "jbm_bold", (void*)jbm_bold, (int)jbm_bold_len, 16.0f, false, false },
        { "jbm_med", (void*)jbm_med, (int)jbm_med_len, 16.0f, false, false },
    };
    return fonts;
}
