#include "menubar.hpp"
#include <constants.hpp>

void CMenuBar::add_group( CMenuBarGroup group ) { m_groups.push_back( std::move( group ) ); }

void CMenuBar::render_item( const CMenuBarItem& item, float btn_h ) {
    bool is_active = item.state && *item.state;

    if ( is_active ) {
        ImVec4 col = item.active_color.value_or( ImGui::GetStyle( ).Colors[ImGuiCol_ButtonActive] );
        ImGui::PushStyleColor( ImGuiCol_Button, col );
        ImGui::PushStyleColor( ImGuiCol_ButtonHovered, col );
        ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.f, 1.f, 1.f, 1.f ) );
    }

    std::string lbl = std::format( "{}\n{}", item.icon, item.label );
    if ( ImGui::Button( lbl.c_str( ), { 0.f, btn_h } ) ) {
        if ( item.state ) *item.state = !*item.state;
        if ( item.callback ) item.callback( );
    }

    if ( is_active ) ImGui::PopStyleColor( 3 );
}

void CMenuBar::render( ) {
    auto& style = ImGui::GetStyle( );
    float font_h = ImGui::GetTextLineHeight( );
    float pad_y = style.FramePadding.y;
    float gap = style.ItemSpacing.y;

    float btn_h = font_h * 2.f + pad_y * 2.f;
    float label_y = btn_h + gap;
    float total_h = label_y + font_h + gap;

    ImGui::BeginChild(
        "##menubar", { 0.f, total_h }, ImGuiChildFlags_None,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse );

    ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.f, 0.f, 0.f, 0.f ) );
    ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 0.f, 0.f, 0.f, 0.f ) );
    ImGui::PushStyleColor( ImGuiCol_ButtonActive, ImVec4( 0.f, 0.f, 0.f, 0.f ) );
    ImGui::Button( APP_NAME.data(), { 0.f, btn_h } );
    ImGui::PopStyleColor( 3 );
    ImGui::SameLine( 0, 16.f );

    ImVec2 sp = ImGui::GetCursorScreenPos( );
    ImGui::GetWindowDrawList( )->AddLine(
        { sp.x, sp.y + 2.f }, { sp.x, sp.y + btn_h - 2.f }, ImGui::GetColorU32( ImGuiCol_Separator ) );
    ImGui::Dummy( { 1.f, btn_h } );
    ImGui::SameLine( 0, 16.f );

    struct GroupBounds {
            float x_start, x_end;
    };
    std::vector<GroupBounds> bounds;
    bounds.reserve( m_groups.size( ) );

    bool first_group = true;
    for ( auto& group : m_groups ) {
        if ( !first_group ) {
            ImGui::SameLine( 0, 8.f );
            ImVec2 sep = ImGui::GetCursorScreenPos( );
            ImGui::GetWindowDrawList( )->AddLine(
                { sep.x, sep.y + 2.f }, { sep.x, sep.y + btn_h - 2.f }, ImGui::GetColorU32( ImGuiCol_Separator ) );
            ImGui::Dummy( { 1.f, btn_h } );
            ImGui::SameLine( 0, 8.f );
        }

        float x_start = 0.f, x_end = 0.f;
        bool first_item = true;
        for ( auto& item : group.items ) {
            if ( !first_item ) ImGui::SameLine( 0, 4.f );
            render_item( item, btn_h );
            if ( first_item ) x_start = ImGui::GetItemRectMin( ).x - ImGui::GetWindowPos( ).x;
            x_end = ImGui::GetItemRectMax( ).x - ImGui::GetWindowPos( ).x;
            first_item = false;
        }
        bounds.push_back( { x_start, x_end } );
        first_group = false;
    }

    for ( size_t i = 0; i < m_groups.size( ); i++ ) {
        float lbl_w = ImGui::CalcTextSize( m_groups[i].label ).x;
        float group_w = bounds[i].x_end - bounds[i].x_start;
        float lbl_x = bounds[i].x_start + ( group_w - lbl_w ) * 0.5f;
        ImGui::SetCursorPos( { std::max( bounds[i].x_start, lbl_x ), label_y } );
        ImGui::TextDisabled( "%s", m_groups[i].label );
    }

    ImGui::SetCursorPosY( label_y + font_h + gap * 0.5f );
    ImGui::Separator( );

    ImGui::EndChild( );
}
