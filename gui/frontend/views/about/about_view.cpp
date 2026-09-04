#include "about_view.hpp"
#include <branding.hpp>
#include <version.hpp>

#include <backend/font_manager/font_manager.hpp>

#include <frontend/childguard.hpp>
#include <frontend/smlogo.hpp>
#include <frontend/ui.hpp>

void CAboutView::on_enter( ) {
    m_embedded = CImageManager::get( ).load_from_memory( smlogo_data, smlogo_len, "logo" );
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
    ImGui::Text( "%s", APP_VERSION );
    ImGui::Text( "Build date" );
    ImGui::SameLine( 120.0f );
    ImGui::Text( "%s", build_date );
    ImGui::Text( "Commit hash" );
    ImGui::SameLine( 120.0f );
    ImGui::TextLinkOpenURL(
        git_commit, std::format( "https://github.com/msh31/SaveManager/commit/{}", git_commit ).c_str( ) );

    ImGui::Text( "Author" );
    ImGui::SameLine( 120.0f );
    ImGui::Text( "%s", APP_AUTHOR );
    ImGui::Text( "License" );
    ImGui::SameLine( 120.0f );
    ImGui::Text( "GPLv3" );
    ImGui::Text( "Source" );
    ImGui::SameLine( 120.0f );
    ImGui::TextLinkOpenURL( "click for sauce", "https://github.com/msh31/SaveManager" );
    ImGui::EndGroup( );

    ImGui::SameLine( win_width - 256.0f - 20.0f );
    {
        ImGui::PushStyleVar( ImGuiStyleVar_ChildBorderSize, { 0.0f } );
        ChildGuard contain( "logo", { 256.f, 256.f } );
        auto p_min = ImGui::GetCursorScreenPos( );
        auto size = ImGui::GetContentRegionAvail( );

        ui::add_contain_image(
            ImGui::GetWindowDrawList( ), (ImTextureID)m_embedded.texture_id, p_min,
            ImVec2( p_min.x + size.x, p_min.y + size.y ), (float)m_embedded.texture_width,
            (float)m_embedded.texture_height );

        ImGui::Dummy( size );
        ImGui::PopStyleVar( );
    }
}

void CAboutView::on_exit( ) {}
