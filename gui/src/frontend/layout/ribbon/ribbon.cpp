#include "ribbon.hpp"

void CRibbonShell::add_nav_item( CBaseView::ViewItem item ) { m_items.push_back( item ); }

CBaseView* CRibbonShell::render( CBaseView* active ) {
    CBaseView* result = nullptr;

    ImGui::PushStyleVar( ImGuiStyleVar_FrameRounding, 0.f );
    for ( auto& item : m_items ) {
        bool is_active = item.view == active;
        if ( is_active ) {
            ImGui::PushStyleColor( ImGuiCol_Button, ImGui::GetStyle( ).Colors[ImGuiCol_ButtonActive] );
            ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.f, 1.f, 1.f, 1.f ) );
        }
        std::string label = std::format( "{}  {}##ribbon_{}", item.icon, item.label, item.label );
        if ( ImGui::Button( label.c_str( ) ) ) result = item.view;
        if ( is_active ) ImGui::PopStyleColor( 2 );
        ImGui::SameLine( );
    }
    ImGui::NewLine( );
    ImGui::Separator( );
    ImGui::PopStyleVar( );

    ImGui::BeginChild( "##content", { 0, 0 }, ImGuiChildFlags_Borders );
    if ( active ) active->render( );
    ImGui::EndChild( );

    return result;
}
