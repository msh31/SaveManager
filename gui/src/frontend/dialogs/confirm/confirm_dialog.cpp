#include "confirm_dialog.hpp"

constexpr float btn_width = 80.f;

void ConfirmDialog::render( ) {
    if ( is_open ) {
        ImGui::OpenPopup( message.c_str( ) );
        is_open = false;
    }

    bool p_open = true;

    float spacing = ImGui::GetStyle( ).ItemSpacing.x;
    float total = btn_width * 2 + spacing;

    ImVec2 center = ImGui::GetMainViewport( )->GetCenter( );
    ImGui::SetNextWindowPos( center, ImGuiCond_Always, { 0.5f, 0.5f } );
    ImGui::SetNextWindowSize( { 200, 75 } );
    if ( ImGui::BeginPopupModal( message.c_str( ), &p_open, ImGuiWindowFlags_NoResize ) ) {
        ImGui::SetCursorPosX( ( ImGui::GetWindowWidth( ) - total ) * 0.5f );
        if ( ImGui::Button( "Yes", { btn_width, 0 } ) ) {
            fun( );
            ImGui::CloseCurrentPopup( );
        }
        ImGui::SameLine( );
        if ( ImGui::Button( "No", { btn_width, 0 } ) ) ImGui::CloseCurrentPopup( );
        ImGui::EndPopup( );
    }
}

void ConfirmDialog::show( std::string str, std::function<void( )> callback ) {
    message = str;
    fun = callback;
    is_open = true;
}
