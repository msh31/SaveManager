#include "log_view.hpp"
#include <frontend/notification/notification.hpp>

void CLogView::render( ) {
    auto* sink = get_ringbuffer_sink( );
    if ( !sink ) {
        ImGui::TextDisabled( "Log sink not initialised." );
        return;
    }

    auto messages = sink->get_messages( );

    ImGui::Checkbox( "Auto-scroll", &m_auto_scroll );
    ImGui::SameLine( );
    if ( ImGui::Button( "Clear" ) ) sink->clear( );
    ImGui::SameLine( );
    if ( ImGui::Button( "Copy to clipboard" ) ) {
        if ( !messages.empty( ) ) {
            std::string all;
            for ( const auto& m : messages ) {
                all += m;
                all += '\n';
            }
            ImGui::SetClipboardText( all.c_str( ) );
            Notify::show_notification( "Logging", "Copied log to clipboard!", 2000 );
        }
    }
    ImGui::Separator( );

    ImGui::BeginChild( "##log_scroll", { 0.f, 0.f }, ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar );

    for ( const auto& msg : messages ) {
        ImVec4 col = ImGui::GetStyleColorVec4( ImGuiCol_Text );
        if ( msg.find( "[error]" ) != std::string::npos || msg.find( "[critical]" ) != std::string::npos )
            col = ImColor( 255, 80, 80 ).Value;
        else if ( msg.find( "[warning]" ) != std::string::npos )
            col = ImColor( 255, 200, 50 ).Value;
        else if ( msg.find( "[debug]" ) != std::string::npos )
            col = ImColor( 130, 130, 130 ).Value;

        ImGui::TextColored( col, "%s", msg.c_str( ) );
    }

    if ( m_auto_scroll && ImGui::GetScrollY( ) >= ImGui::GetScrollMaxY( ) - 8.f ) ImGui::SetScrollHereY( 1.f );

    ImGui::EndChild( );
}
