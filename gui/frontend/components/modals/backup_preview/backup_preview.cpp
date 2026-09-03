#include "backup_preview.hpp"
#include <utils/utils.hpp>
#include <frontend/notification/notification.hpp>

void CBackupPreviewModal::open( const std::vector<std::string>& list) {
    m_preview_list = list;
    request_open( );
}

void CBackupPreviewModal::render_content( ) {
    int i = 0;
    for ( const auto& entry : m_preview_list ) {
        ImGui::PushID( entry.c_str( ) );
        i += 1;
        std::string text = std::format( "{}: {}", i, utils::path_to_utf8( utils::utf8_to_path( entry ).filename( ) ) );
        ImGui::Text( "%s", text.c_str( ) );
        ImGui::Separator( );
        ImGui::PopID( );
    }

    if ( ImGui::Button( "OK" ) ) ImGui::CloseCurrentPopup( );
}