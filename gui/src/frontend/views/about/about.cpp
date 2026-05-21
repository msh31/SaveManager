#include "about.hpp"
#include <constants.hpp>
#include <version.hpp>

void AboutTab::render( const Fonts &fonts ) {
    ImGui::NewLine( );

    float win_width = ImGui::GetWindowSize( ).x;

    ImGui::PushFont( fonts.title );
    float title_width = ImGui::CalcTextSize( "SaveManager" ).x;
    ImGui::SetCursorPosX( ( win_width - title_width ) * 0.5f );
    ImGui::Text( "SaveManager" );
    ImGui::PopFont( );

    ImGui::PushFont( fonts.medium );
    float subtitle_width = ImGui::CalcTextSize( subtitle ).x;
    ImGui::SetCursorPosX( ( win_width - subtitle_width ) * 0.5f );
    ImGui::TextDisabled( "%s", subtitle );
    ImGui::PopFont( );

    ImGui::Dummy( ImVec2( 0.0f, 4.0f ) );
    ImGui::Separator( );
    ImGui::Dummy( ImVec2( 0.0f, 4.0f ) );

    ImGui::PushFont( fonts.header );
    ImGui::Text( "Details" );
    ImGui::PopFont( );

    ImGui::Text( "Version" );
    ImGui::SameLine( 120.0f );
    ImGui::Text( APP_VERSION );
    ImGui::Text( "Build date" );
    ImGui::SameLine( 120.0f );
    ImGui::Text( "%s", build_date );
    ImGui::Text( "Commit hash" );
    ImGui::SameLine( 120.0f );
    ImGui::TextLinkOpenURL( git_commit,
                            std::format( "https://github.com/msh31/SaveManager/commit/{}", git_commit ).c_str( ) );

    ImGui::Text( "Author" );
    ImGui::SameLine( 120.0f );
    ImGui::Text( APP_AUTHOR );
    ImGui::Text( "License" );
    ImGui::SameLine( 120.0f );
    ImGui::Text( "GPLv3" );
    ImGui::Text( "Source" );
    ImGui::SameLine( 120.0f );
    ImGui::TextLinkOpenURL( "click for sauce", "https://github.com/msh31/SaveManager" );
}
