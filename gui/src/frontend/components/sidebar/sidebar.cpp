#include "sidebar.hpp"
#include <constants.hpp>

CBaseView* CSideBar::render( CBaseView* active ) {
    ImGui::BeginChild( "##sidebar", { 180.f, 0 }, ImGuiChildFlags_Borders );
    float content_w = ImGui::GetContentRegionAvail( ).x;
    float btn_h     = ImGui::GetFrameHeight( );

    ImGui::TextDisabled( APP_NAME );
    ImGui::Separator( );
    ImGui::Spacing( );

    CBaseView* r_item = nullptr;
    for ( const auto& item : m_items ) {
        if ( nav_button( item.icon, item.label, item.view == active, content_w ) ) {
            r_item = item.view;
            break;
        }
    }

    ImGui::SetCursorPosY( ImGui::GetWindowHeight( ) - btn_h - ImGui::GetStyle( ).WindowPadding.y );
    ImGui::PushStyleVar( ImGuiStyleVar_ButtonTextAlign, ImVec2( 0.0f, 0.5f ) );
    if ( ImGui::Button( "  \xef\x80\x93   Settings", ImVec2( content_w, 0 ) ) ) {
        r_item = m_settings;
    }
    ImGui::PopStyleVar( );
    ImGui::EndChild( );
    return r_item;
}

void CSideBar::add_item( CBaseView::ViewItem item ) { m_items.push_back( item ); }

void CSideBar::set_settings_view( CBaseView* view ) { m_settings = view; }

bool CSideBar::nav_button( const char* icon, const char* label, bool active, float width ) {
    ImGuiStyle& style = ImGui::GetStyle( );

    if ( active ) {
        ImGui::PushStyleColor( ImGuiCol_Button, style.Colors[ImGuiCol_ButtonActive] );
        ImGui::PushStyleColor( ImGuiCol_ButtonHovered, style.Colors[ImGuiCol_ButtonActive] );
        ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
    }
    ImGui::PushStyleVar( ImGuiStyleVar_ButtonTextAlign, ImVec2( 0.0f, 0.5f ) );

    std::string text    = std::format( "  {}   {}##nav_{}", icon, label, label );
    bool        clicked = ImGui::Button( text.c_str( ), ImVec2( width, 0 ) );

    ImGui::PopStyleVar( );
    if ( active ) ImGui::PopStyleColor( 3 );
    return clicked;
}
