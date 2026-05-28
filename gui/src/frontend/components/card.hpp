#pragma once
#include <backend/font_manager/font_manager.hpp>

namespace Card {
    template <typename F>
    inline void draw(
        std::string_view id, std::string_view text, bool& expanded, std::optional<std::string_view> r_text,
        F&& content ) {
        const char* chevron = expanded ? "▼" : "▶";
        auto        label   = std::format( "##{}", id.data( ) );

        ImGui::PushStyleColor( ImGuiCol_Border, ImVec4( 198 / 255.f, 97 / 255.f, 63 / 255.f, 1.f ) );
        ImGui::PushStyleVar( ImGuiStyleVar_ChildRounding, 4.0f );
        ImGui::BeginChild( id.data( ), ImVec2( 0, 0 ), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY );
        ImGui::PopStyleColor( );

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

        if ( expanded ) {
            content( );
        }

        ImGui::EndChild( );
        ImGui::PopStyleVar( );
    }
} // namespace Card
