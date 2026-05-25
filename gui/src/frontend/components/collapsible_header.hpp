#pragma once
#include <backend/font_manager/font_manager.hpp>

namespace CollapsibleHeader {
    inline void
    draw( std::string_view id, std::string_view text, bool& expanded, std::optional<std::string_view> r_text ) {
        const char* chevron = expanded ? "▼" : "▶";
        auto        label   = std::format( "##{}", id.data( ) );

        if ( ImGui::Selectable( label.c_str( ), false, ImGuiSelectableFlags_None, ImVec2( 0, 30 ) ) ) {
            expanded = !expanded;
        }
        ImGui::SameLine( 8.0f );

        ImGui::PushFont( CFontManager::get( ).get_font( "jbm_bold" ).value_or( nullptr ) );
        ImGui::TextColored( ImColor( 198, 97, 63 ).Value, "%s", chevron );
        ImGui::PopFont( );
        ImGui::SameLine( );

        ImGui::PushFont( CFontManager::get( ).get_font( "jbm_med" ).value_or( nullptr ) );
        ImGui::Text( "%s", text.data( ) );
        ImGui::PopFont( );

        if ( r_text.has_value( ) ) {
            ImGui::SameLine( ImGui::GetContentRegionMax( ).x - ImGui::CalcTextSize( r_text->data( ) ).x );
            ImGui::Text( "%s", r_text->data( ) );
        }
    }
} // namespace CollapsibleHeader
