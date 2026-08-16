#include "about_view.hpp"
#include <constants.hpp>
#include <version.hpp>

#include <backend/font_manager/font_manager.hpp>
#include <frontend/smlogo.hpp>

// #define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void CAboutView::on_enter( ) {
    if ( m_logo_loaded ) return;

    int width = 0, height = 0, channels = 4;
    auto* pixels = stbi_load_from_memory( smlogo_data, smlogo_len, &width, &height, &channels, 4 );

    glGenTextures( 1, &m_logo_tex );
    glBindTexture( GL_TEXTURE_2D, m_logo_tex );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels );

    stbi_image_free( pixels );
    m_logo_loaded = true;
}

void CAboutView::render( ) {
    static const char* subtitle = "The swiss army knife of save management";

    ImGui::NewLine( );
    float win_width = ImGui::GetWindowSize( ).x;

    ImGui::BeginGroup( );
    ImGui::PushFont( CFontManager::get( ).get_font( "jbm_title" ).value_or( nullptr ) );
    ImGui::Text( "SaveManager" );
    ImGui::PopFont( );
    ImGui::PushFont( CFontManager::get( ).get_font( "jbm_med" ).value_or( nullptr ) );
    ImGui::TextDisabled( "%s", subtitle );
    ImGui::PopFont( );
    ImGui::EndGroup( );

    ImGui::Dummy( ImVec2( 0.0f, 4.0f ) );
    ImGui::Separator( );
    ImGui::Dummy( ImVec2( 0.0f, 4.0f ) );

    ImGui::BeginGroup( );
    ImGui::PushFont( CFontManager::get( ).get_font( "jbm_header" ).value_or( nullptr ) );
    ImGui::Text( "Details" );
    ImGui::PopFont( );

    ImGui::Text( "Version" );
    ImGui::SameLine( 120.0f );
    ImGui::Text( "%s", APP_VERSION.data( ) );
    ImGui::Text( "Build date" );
    ImGui::SameLine( 120.0f );
    ImGui::Text( "%s", build_date );
    ImGui::Text( "Commit hash" );
    ImGui::SameLine( 120.0f );
    ImGui::TextLinkOpenURL(
        git_commit, std::format( "https://github.com/msh31/SaveManager/commit/{}", git_commit ).c_str( ) );

    ImGui::Text( "Author" );
    ImGui::SameLine( 120.0f );
    ImGui::Text( "%s", APP_AUTHOR.data( ) );
    ImGui::Text( "License" );
    ImGui::SameLine( 120.0f );
    ImGui::Text( "GPLv3" );
    ImGui::Text( "Source" );
    ImGui::SameLine( 120.0f );
    ImGui::TextLinkOpenURL( "click for sauce", "https://github.com/msh31/SaveManager" );
    ImGui::EndGroup( );

    ImGui::SameLine( win_width - 256.0f - 20.0f );
    ImGui::Image( (ImTextureRef)(intptr_t)m_logo_tex, ImVec2( 256, 256 ) );
}

void CAboutView::on_exit( ) {}
