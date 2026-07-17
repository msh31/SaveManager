#include "tabbar.hpp"

void CTabbarShell::add_nav_item( CBaseView::ViewItem item ) { m_items.push_back( item ); }

CBaseView* CTabbarShell::render( CBaseView* active ) {
    CBaseView* result = nullptr;

#ifndef NDEBUG
    ImGui::TextColored( ImColor( 220, 40, 30, 255 ), "DEBUG" );
#endif // NDEBUG

    if ( ImGui::BeginTabBar( "##navtabs" ) ) {
        for ( auto& item : m_items ) {
            ImGuiTabItemFlags flags = ( item.view == active ) ? ImGuiTabItemFlags_SetSelected : 0;
            std::string tab_name = std::format( "{} {}", item.icon, item.label );

            if ( ImGui::BeginTabItem( tab_name.c_str( ), nullptr, flags ) ) ImGui::EndTabItem( );
            if ( ImGui::IsItemClicked( ) ) result = item.view;
        }
        ImGui::EndTabBar( );
    }

    ImGui::BeginChild( "##content", { 0, 0 }, ImGuiChildFlags_Borders, ImGuiWindowFlags_NoBackground );
    if ( active ) active->render( );
    ImGui::EndChild( );

    return result;
}
