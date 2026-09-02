#pragma once

namespace ui {
    // text
    inline void add_font_text( const std::string& text, ImFont* font ) {
        ImGui::PushFont( font );
        ImGui::Text( "%s", text.c_str() );
        ImGui::PopFont( );
    }

    inline void add_styled_text( const std::string& text, ImGuiStyleVar_ style, float style_size ) {
        ImGui::PushStyleVar( style, style_size );
        ImGui::Text( "%s", text.c_str( ) );
        ImGui::PopStyleVar( );
    }

    // buttons
    inline void add_danger_button( const std::string& text, ImVec2 size, std::function<void( )> fun ) {
        ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.8f, 0.2f, 0.2f, 1.0f ) );
        ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 0.9f, 0.3f, 0.3f, 1.0f ) );

        if ( ImGui::Button( text.c_str( ), size ) ) fun( );
        ImGui::PopStyleColor( 2 );
    }

    inline void add_small_button( const std::string& text, std::function<void( )> fun ) {
        ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 3.0f, 3.0f ) );
        if ( ImGui::Button( text.c_str( ) ) ) fun( );
        ImGui::PopStyleVar( );
    }

    // Two ways to fit an image into a rect it doesn't share an aspect ratio with.
    // add_cover_image:   fills the rect completely, no distortion, crops whatever overflows (CSS background-size:
    // cover). add_contain_image: shows the whole image with no distortion or cropping, letterboxed within the rect (CSS
    // background-size: contain).
    inline void add_cover_image(
        ImDrawList* draw_list, ImTextureID texture_id, ImVec2 p_min, ImVec2 p_max, float image_w, float image_h ) {
        if ( image_w <= 0.0f || image_h <= 0.0f ) return;

        float dest_w = p_max.x - p_min.x;
        float dest_h = p_max.y - p_min.y;
        if ( dest_w <= 0.0f || dest_h <= 0.0f ) return;

        float image_aspect = image_w / image_h;
        float dest_aspect = dest_w / dest_h;

        ImVec2 uv_min( 0.0f, 0.0f ), uv_max( 1.0f, 1.0f );
        if ( image_aspect > dest_aspect ) {
            float visible_fraction = dest_aspect / image_aspect;
            uv_min.x = ( 1.0f - visible_fraction ) * 0.5f;
            uv_max.x = 1.0f - uv_min.x;
        } else if ( image_aspect < dest_aspect ) {
            float visible_fraction = image_aspect / dest_aspect;
            uv_min.y = ( 1.0f - visible_fraction ) * 0.5f;
            uv_max.y = 1.0f - uv_min.y;
        }

        draw_list->AddImage( texture_id, p_min, p_max, uv_min, uv_max );
    }

    inline void add_contain_image(
        ImDrawList* draw_list, ImTextureID texture_id, ImVec2 p_min, ImVec2 p_max, float image_w, float image_h ) {
        if ( image_w <= 0.0f || image_h <= 0.0f ) return;

        float dest_w = p_max.x - p_min.x;
        float dest_h = p_max.y - p_min.y;
        if ( dest_w <= 0.0f || dest_h <= 0.0f ) return;

        float scale = std::min( dest_w / image_w, dest_h / image_h );
        float draw_w = image_w * scale;
        float draw_h = image_h * scale;

        ImVec2 origin( p_min.x + ( dest_w - draw_w ) * 0.5f, p_min.y + ( dest_h - draw_h ) * 0.5f );
        draw_list->AddImage( texture_id, origin, ImVec2( origin.x + draw_w, origin.y + draw_h ) );
    }
} // namespace ui
