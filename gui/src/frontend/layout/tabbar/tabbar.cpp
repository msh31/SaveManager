#include "tabbar.hpp"

void CTabbarShell::add_nav_item( CBaseView::ViewItem item ) { m_items.push_back( item ); }

CBaseView* CTabbarShell::render( CBaseView* active ) {
    CBaseView* result = nullptr;

    if ( ImGui::BeginTabBar( "##navtabs" ) ) {
        for ( auto& item : m_items ) {
            ImGuiTabItemFlags flags = ( item.view == active ) ? ImGuiTabItemFlags_SetSelected : 0;
            if ( ImGui::BeginTabItem( item.label, nullptr, flags ) ) ImGui::EndTabItem( );
            if ( ImGui::IsItemClicked( ) ) result = item.view;
        }
        ImGui::EndTabBar( );
    }

    ImGui::BeginChild( "##content", { 0, 0 }, ImGuiChildFlags_Borders );
    if ( active ) active->render( );
    ImGui::EndChild( );

    return result;
}
