#include "sidebar.hpp"

void CSidebarShell::add_nav_item( CBaseView::ViewItem item ) { m_sidebar.add_item( item ); }

void CSidebarShell::set_settings( CBaseView::ViewItem item ) { m_sidebar.set_settings_view( item.view ); }

CBaseView* CSidebarShell::render( CBaseView* active ) {
    CBaseView* clicked = m_sidebar.render( active );

    ImGui::SameLine( );
    ImGui::BeginChild( "##content", { 0, 0 }, ImGuiChildFlags_Borders );
    if ( active ) active->render( );
    ImGui::EndChild( );

    return clicked;
}
