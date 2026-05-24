#pragma once

namespace ui {
    // text
    inline void add_colored_text( std::string_view text, ImVec4 color ) {
        ImGui::PushStyleColor( ImGuiCol_Text, color );
        ImGui::Text( "%s", text.data( ) );
        ImGui::PopStyleColor( );

        // hmmm, this exists too.
        //  ImGui::TextColored(ImColor(198, 97, 63).Value, "%s", std::string(text).c_str());
    }

    inline void add_font_text( std::string_view text, ImFont* font, float size = 0.0f ) {
        ImGui::PushFont( font, size );
        ImGui::Text( "%s", text.data( ) );
        ImGui::PopFont( );
    }

    inline void add_styled_text( std::string_view text, ImGuiStyleVar_ style, float style_size ) {
        ImGui::PushStyleVar( style, style_size );
        ImGui::Text( "%s", std::string( text ).c_str( ) );
        ImGui::PopStyleVar( );
    }

    // buttons
    inline void add_danger_button( std::string_view text, ImVec2 size, std::function<void( )> fun ) {
        ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.8f, 0.2f, 0.2f, 1.0f ) );
        ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 0.9f, 0.3f, 0.3f, 1.0f ) );

        if ( ImGui::Button( text.data( ), size ) ) fun( );
        ImGui::PopStyleColor( 2 );
    }

    inline void add_small_button( std::string_view text, std::function<void( )> fun ) {
        ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 3.0f, 3.0f ) );
        if ( ImGui::Button( text.data( ) ) ) fun( );
        ImGui::PopStyleVar( );
    }
} // namespace ui
