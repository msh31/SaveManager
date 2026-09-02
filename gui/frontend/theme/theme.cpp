/*
 *  Also stolen from SaveManager ->
 * https://git.marco007.dev/marco/SaveManager/src/branch/main/src/frontend/ui/themes/themes.cpp
 */

#include "theme.hpp"
#include <logger.hpp>

void ThemeManager::apply_style( ) {
    style = ImGui::GetStyle( );
    style.WindowPadding = { 12.f, 12.f };
    style.PopupRounding = 8.f;
    style.FramePadding = { 10.f, 6.f };
    style.ItemSpacing = { 10.f, 8.f };
    style.ItemInnerSpacing = { 8.f, 6.f };
    style.TouchExtraPadding = { 0.f, 0.f };
    style.IndentSpacing = 21.f;
    style.ScrollbarSize = 12.f;
    style.GrabMinSize = 12.f;
    style.WindowBorderSize = 0.f;
    style.ChildBorderSize = 1.f;
    style.PopupBorderSize = 1.f;
    style.FrameBorderSize = 1.f;
    style.TabBorderSize = 0.f;
    style.WindowRounding = 8.f;
    style.ChildRounding = 8.f;
    style.FrameRounding = 6.f;
    style.ScrollbarRounding = 8.f;
    style.GrabRounding = 12.f;
    style.TabRounding = 8.f;
    style.WindowTitleAlign = { 0.5f, 0.5f };
    style.ButtonTextAlign = { 0.5f, 0.5f };
    style.DisplaySafeAreaPadding = { 3.f, 3.f };

    ImGui::GetStyle( ) = style;
}

void ThemeManager::apply_scale( float scale ) {
    ImGuiStyle scaled = style;
    scaled.ScaleAllSizes( scale );
    std::copy( std::begin( ImGui::GetStyle( ).Colors ), std::end( ImGui::GetStyle( ).Colors ), scaled.Colors );
    ImGui::GetStyle( ) = scaled;
}

void ThemeManager::apply_colors( ThemeType theme ) {
    if ( has_applied && theme == current_theme ) return;
    current_theme = theme;
    has_applied = true;

    auto& colors = ImGui::GetStyle( ).Colors;
    switch ( theme ) {
    case ThemeType::Dark:
    case ThemeType::Custom:
        colors[ImGuiCol_Text] = ImVec4( 0.90f, 0.90f, 0.90f, 1.00f );
        colors[ImGuiCol_TextDisabled] = ImVec4( 0.45f, 0.45f, 0.45f, 1.00f );
        colors[ImGuiCol_WindowBg] = ImVec4( 0.06f, 0.06f, 0.06f, 1.00f );
        colors[ImGuiCol_ChildBg] = ImVec4( 0.08f, 0.08f, 0.08f, 1.00f );
        colors[ImGuiCol_PopupBg] = ImVec4( 0.09f, 0.09f, 0.09f, 0.97f );
        colors[ImGuiCol_Border] = ImVec4( 0.30f, 0.30f, 0.30f, 0.50f );
        colors[ImGuiCol_BorderShadow] = ImVec4( 0.00f, 0.00f, 0.00f, 0.00f );
        colors[ImGuiCol_FrameBg] = ImVec4( 0.13f, 0.13f, 0.13f, 1.00f );
        colors[ImGuiCol_FrameBgHovered] = ImVec4( 0.17f, 0.17f, 0.17f, 1.00f );
        colors[ImGuiCol_FrameBgActive] = ImVec4( 0.21f, 0.21f, 0.21f, 1.00f );
        colors[ImGuiCol_CheckboxSelectedBg] = ImVec4( 0.86f, 0.08f, 0.24f, 0.25f );
        colors[ImGuiCol_TitleBg] = ImVec4( 0.06f, 0.06f, 0.06f, 1.00f );
        colors[ImGuiCol_TitleBgActive] = ImVec4( 0.09f, 0.09f, 0.09f, 1.00f );
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4( 0.00f, 0.00f, 0.00f, 0.51f );
        colors[ImGuiCol_MenuBarBg] = ImVec4( 0.06f, 0.06f, 0.06f, 1.00f );
        colors[ImGuiCol_ScrollbarBg] = ImVec4( 0.06f, 0.06f, 0.06f, 0.00f );
        colors[ImGuiCol_ScrollbarGrab] = ImVec4( 0.26f, 0.26f, 0.26f, 1.00f );
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4( 0.33f, 0.33f, 0.33f, 1.00f );
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4( 0.40f, 0.40f, 0.40f, 1.00f );
        colors[ImGuiCol_CheckMark] = ImVec4( 0.86f, 0.08f, 0.24f, 1.00f );
        colors[ImGuiCol_SliderGrab] = ImVec4( 0.86f, 0.08f, 0.24f, 1.00f );
        colors[ImGuiCol_SliderGrabActive] = ImVec4( 0.96f, 0.28f, 0.40f, 1.00f );
        colors[ImGuiCol_Button] = ImVec4( 0.14f, 0.14f, 0.14f, 1.00f );
        colors[ImGuiCol_ButtonHovered] = ImVec4( 0.19f, 0.19f, 0.19f, 1.00f );
        colors[ImGuiCol_ButtonActive] = ImVec4( 0.64f, 0.05f, 0.18f, 1.00f );
        colors[ImGuiCol_Header] = ImVec4( 0.86f, 0.08f, 0.24f, 0.30f );
        colors[ImGuiCol_HeaderHovered] = ImVec4( 0.86f, 0.08f, 0.24f, 0.45f );
        colors[ImGuiCol_HeaderActive] = ImVec4( 0.86f, 0.08f, 0.24f, 0.60f );
        colors[ImGuiCol_Separator] = ImVec4( 0.30f, 0.30f, 0.30f, 0.50f );
        colors[ImGuiCol_SeparatorHovered] = ImVec4( 0.96f, 0.28f, 0.40f, 0.60f );
        colors[ImGuiCol_SeparatorActive] = ImVec4( 0.86f, 0.08f, 0.24f, 0.80f );
        colors[ImGuiCol_ResizeGrip] = ImVec4( 0.86f, 0.08f, 0.24f, 0.20f );
        colors[ImGuiCol_ResizeGripHovered] = ImVec4( 0.86f, 0.08f, 0.24f, 0.45f );
        colors[ImGuiCol_ResizeGripActive] = ImVec4( 0.86f, 0.08f, 0.24f, 0.65f );
        colors[ImGuiCol_Tab] = ImVec4( 0.09f, 0.09f, 0.09f, 1.00f );
        colors[ImGuiCol_TabHovered] = ImVec4( 0.16f, 0.16f, 0.16f, 1.00f );
        colors[ImGuiCol_TabActive] = ImVec4( 0.13f, 0.13f, 0.13f, 1.00f );
        colors[ImGuiCol_TabUnfocused] = ImVec4( 0.06f, 0.06f, 0.06f, 1.00f );
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4( 0.09f, 0.09f, 0.09f, 1.00f );
        colors[ImGuiCol_TabSelectedOverline] = ImVec4( 0.86f, 0.08f, 0.24f, 1.00f );
        colors[ImGuiCol_PlotLines] = ImVec4( 0.60f, 0.60f, 0.60f, 1.00f );
        colors[ImGuiCol_PlotLinesHovered] = ImVec4( 0.96f, 0.28f, 0.40f, 1.00f );
        colors[ImGuiCol_PlotHistogram] = ImVec4( 0.86f, 0.08f, 0.24f, 1.00f );
        colors[ImGuiCol_PlotHistogramHovered] = ImVec4( 0.96f, 0.28f, 0.40f, 1.00f );
        colors[ImGuiCol_TextSelectedBg] = ImVec4( 0.86f, 0.08f, 0.24f, 0.35f );
        colors[ImGuiCol_DragDropTarget] = ImVec4( 0.96f, 0.28f, 0.40f, 0.90f );
        colors[ImGuiCol_NavHighlight] = ImVec4( 0.86f, 0.08f, 0.24f, 1.00f );
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4( 1.00f, 1.00f, 1.00f, 0.70f );
        colors[ImGuiCol_NavWindowingDimBg] = ImVec4( 0.80f, 0.80f, 0.80f, 0.20f );
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4( 0.00f, 0.00f, 0.00f, 0.50f );
        break;
    case ThemeType::Light:
        colors[ImGuiCol_Text] = ImVec4( 0.10f, 0.10f, 0.10f, 1.00f );
        colors[ImGuiCol_TextDisabled] = ImVec4( 0.55f, 0.55f, 0.55f, 1.00f );
        colors[ImGuiCol_WindowBg] = ImVec4( 0.98f, 0.98f, 0.98f, 1.00f );
        colors[ImGuiCol_ChildBg] = ImVec4( 0.96f, 0.96f, 0.96f, 1.00f );
        colors[ImGuiCol_PopupBg] = ImVec4( 0.99f, 0.99f, 0.99f, 0.98f );
        colors[ImGuiCol_Border] = ImVec4( 0.80f, 0.80f, 0.80f, 0.80f );
        colors[ImGuiCol_BorderShadow] = ImVec4( 0.00f, 0.00f, 0.00f, 0.00f );
        colors[ImGuiCol_FrameBg] = ImVec4( 0.93f, 0.93f, 0.93f, 1.00f );
        colors[ImGuiCol_FrameBgHovered] = ImVec4( 0.89f, 0.89f, 0.89f, 1.00f );
        colors[ImGuiCol_FrameBgActive] = ImVec4( 0.85f, 0.85f, 0.85f, 1.00f );
        colors[ImGuiCol_CheckboxSelectedBg] = ImVec4( 0.86f, 0.08f, 0.24f, 0.15f );
        colors[ImGuiCol_TitleBg] = ImVec4( 0.96f, 0.96f, 0.96f, 1.00f );
        colors[ImGuiCol_TitleBgActive] = ImVec4( 0.93f, 0.93f, 0.93f, 1.00f );
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4( 0.85f, 0.85f, 0.85f, 0.51f );
        colors[ImGuiCol_MenuBarBg] = ImVec4( 0.98f, 0.98f, 0.98f, 1.00f );
        colors[ImGuiCol_ScrollbarBg] = ImVec4( 0.98f, 0.98f, 0.98f, 0.00f );
        colors[ImGuiCol_ScrollbarGrab] = ImVec4( 0.78f, 0.78f, 0.78f, 1.00f );
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4( 0.68f, 0.68f, 0.68f, 1.00f );
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4( 0.58f, 0.58f, 0.58f, 1.00f );
        colors[ImGuiCol_CheckMark] = ImVec4( 0.70f, 0.05f, 0.18f, 1.00f );
        colors[ImGuiCol_SliderGrab] = ImVec4( 0.86f, 0.08f, 0.24f, 1.00f );
        colors[ImGuiCol_SliderGrabActive] = ImVec4( 0.70f, 0.05f, 0.18f, 1.00f );
        colors[ImGuiCol_Button] = ImVec4( 0.91f, 0.91f, 0.91f, 1.00f );
        colors[ImGuiCol_ButtonHovered] = ImVec4( 0.86f, 0.86f, 0.86f, 1.00f );
        colors[ImGuiCol_ButtonActive] = ImVec4( 0.86f, 0.08f, 0.24f, 1.00f );
        colors[ImGuiCol_Header] = ImVec4( 0.86f, 0.08f, 0.24f, 0.15f );
        colors[ImGuiCol_HeaderHovered] = ImVec4( 0.86f, 0.08f, 0.24f, 0.25f );
        colors[ImGuiCol_HeaderActive] = ImVec4( 0.86f, 0.08f, 0.24f, 0.40f );
        colors[ImGuiCol_Separator] = ImVec4( 0.80f, 0.80f, 0.80f, 0.80f );
        colors[ImGuiCol_SeparatorHovered] = ImVec4( 0.86f, 0.08f, 0.24f, 0.60f );
        colors[ImGuiCol_SeparatorActive] = ImVec4( 0.70f, 0.05f, 0.18f, 0.80f );
        colors[ImGuiCol_ResizeGrip] = ImVec4( 0.86f, 0.08f, 0.24f, 0.15f );
        colors[ImGuiCol_ResizeGripHovered] = ImVec4( 0.86f, 0.08f, 0.24f, 0.35f );
        colors[ImGuiCol_ResizeGripActive] = ImVec4( 0.86f, 0.08f, 0.24f, 0.55f );
        colors[ImGuiCol_Tab] = ImVec4( 0.91f, 0.91f, 0.91f, 1.00f );
        colors[ImGuiCol_TabHovered] = ImVec4( 0.86f, 0.86f, 0.86f, 1.00f );
        colors[ImGuiCol_TabActive] = ImVec4( 0.99f, 0.99f, 0.99f, 1.00f );
        colors[ImGuiCol_TabUnfocused] = ImVec4( 0.96f, 0.96f, 0.96f, 1.00f );
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4( 0.93f, 0.93f, 0.93f, 1.00f );
        colors[ImGuiCol_TabSelectedOverline] = ImVec4( 0.70f, 0.05f, 0.18f, 1.00f );
        colors[ImGuiCol_PlotLines] = ImVec4( 0.45f, 0.45f, 0.45f, 1.00f );
        colors[ImGuiCol_PlotLinesHovered] = ImVec4( 0.86f, 0.08f, 0.24f, 1.00f );
        colors[ImGuiCol_PlotHistogram] = ImVec4( 0.86f, 0.08f, 0.24f, 1.00f );
        colors[ImGuiCol_PlotHistogramHovered] = ImVec4( 0.70f, 0.05f, 0.18f, 1.00f );
        colors[ImGuiCol_TextSelectedBg] = ImVec4( 0.86f, 0.08f, 0.24f, 0.25f );
        colors[ImGuiCol_DragDropTarget] = ImVec4( 0.86f, 0.08f, 0.24f, 0.90f );
        colors[ImGuiCol_NavHighlight] = ImVec4( 0.86f, 0.08f, 0.24f, 1.00f );
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4( 0.10f, 0.10f, 0.10f, 0.70f );
        colors[ImGuiCol_NavWindowingDimBg] = ImVec4( 0.40f, 0.40f, 0.40f, 0.20f );
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4( 0.40f, 0.40f, 0.40f, 0.35f );
        break;
    }
}
