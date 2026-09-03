#include "base_model.hpp"

void CModalBase::render( ) {
    if ( m_should_open ) {
        m_should_open = false;
        ImGui::OpenPopup( m_title.c_str( ) );
    }
    if ( ImGui::BeginPopupModal( m_title.c_str( ), nullptr, ImGuiWindowFlags_AlwaysAutoResize ) ) {
        render_content( );
        ImGui::EndPopup( );
    }
}