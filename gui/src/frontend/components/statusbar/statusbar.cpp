#include "statusbar.hpp"

void CStatusBar::add_left( Item item ) { m_left.push_back( item ); }
void CStatusBar::add_right( Item item ) { m_right.push_back( item ); }

void CStatusBar::render_item( const Item& item ) {
    if ( item.dot_color ) {
        ImGui::TextColored( *item.dot_color, "\xe2\x97\x8f" );
        ImGui::SameLine( 0, 4.f );
    }
    ImGui::Text( "%s: %s", item.label, item.value );
}

void CStatusBar::render( ) {
    auto& style = ImGui::GetStyle( );

    ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, { 8.f, ( height( ) - ImGui::GetTextLineHeight( ) ) * 0.5f } );
    ImGui::BeginChild(
        "##statusbar", { 0.f, height( ) }, ImGuiChildFlags_Borders,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse );
    ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, { 12.f, 0.f } );

    float content_y = ImGui::GetCursorPosY( );

    for ( size_t i = 0; i < m_left.size( ); i++ ) {
        if ( i > 0 ) ImGui::SameLine( 0, 12.f );
        render_item( m_left[i] );
    }

    float right_w = 0.f;
    for ( auto& item : m_right ) {
        if ( item.dot_color ) right_w += ImGui::CalcTextSize( "\xe2\x97\x8f " ).x + 4.f;
        right_w += ImGui::CalcTextSize( std::format( "{}: {}", item.label, item.value ).c_str( ) ).x;
        right_w += 12.f;
    }
    right_w -= 12.f;

    float start_x = ImGui::GetWindowWidth( ) - right_w - style.WindowPadding.x * 2.f;
    ImGui::SetCursorPos( { std::max( 0.f, start_x ), content_y } );

    for ( size_t i = 0; i < m_right.size( ); i++ ) {
        if ( i > 0 ) ImGui::SameLine( 0, 12.f );
        render_item( m_right[i] );
    }

    ImGui::PopStyleVar( );
    ImGui::EndChild( );
    ImGui::PopStyleVar( );
}
