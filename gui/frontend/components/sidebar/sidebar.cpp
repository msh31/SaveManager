#include "sidebar.hpp"
#include <backend/font_manager/font_manager.hpp>
#include <branding.hpp>
#include <frontend/childguard.hpp>
#include <frontend/icons.hpp>

CBaseView* CSideBar::render( CBaseView* active ) {
    float width = collapsed ? 60.0f : 275.f;

    ChildGuard sidebar_child( "##sidebar", { width, 0 }, ImGuiChildFlags_Borders );
    float content_w = ImGui::GetContentRegionAvail( ).x;
    float btn_h = ImGui::GetFrameHeight( );
    CBaseView* r_item = nullptr;

    if ( !collapsed ) {
        ImGui::PushID( "uncollapsed_button" );
        if ( ImGui::Button( ICON_MENU ) ) {
            collapsed = !collapsed;
        }
        ImGui::PopID( );
        ImGui::SameLine( );
        ImGui::PushFont( CFontManager::get( ).get_font( "jbm_reg_xl" ).value_or( nullptr ) );
        ImGui::TextDisabled( "%s", APP_NAME );
        ImGui::PopFont( );
    }

    if ( collapsed ) {
        ImGui::PushID( "collapsed_button" );
        if ( ImGui::Button( ICON_MENU ) ) {
            collapsed = !collapsed;
        }
        ImGui::PopID( );
    }

    ImGui::Separator( );
    ImGui::Spacing( );

    for ( const auto& item : m_items ) {
        auto label = item.label;
        if ( collapsed ) label = "";
        if ( nav_button( item.icon, label, item.view == active, ImVec2( content_w, 0 ) ) ) {
            r_item = item.view;
            break;
        }
        if ( collapsed ) ImGui::SetItemTooltip( "%s", item.label );
    }

    auto avail = ImGui::GetContentRegionAvail( );
    ImGui::Dummy( ImVec2( avail.x, avail.y - 40.0f ) );

    ImGui::PushStyleVar( ImGuiStyleVar_ButtonTextAlign, ImVec2( 0.0f, 0.5f ) );
    std::string str = std::format( "{}{}", ICON_GEAR, " Settings" );
    if ( collapsed ) str = std::string( ICON_GEAR );
    if ( ImGui::Button( str.c_str( ), ImVec2( content_w, 0 ) ) ) {
        r_item = m_settings;
    }
    ImGui::PopStyleVar( );

    return r_item;
}

void CSideBar::add_item( CBaseView::ViewItem item ) { m_items.push_back( item ); }

void CSideBar::set_settings_view( CBaseView* view ) { m_settings = view; }

bool CSideBar::nav_button( const char* icon, const char* label, bool active, ImVec2 width ) {
    ImGuiStyle& style = ImGui::GetStyle( );

    if ( active ) {
        ImGui::PushStyleColor( ImGuiCol_Button, style.Colors[ImGuiCol_ButtonActive] );
        ImGui::PushStyleColor( ImGuiCol_ButtonHovered, style.Colors[ImGuiCol_ButtonActive] );
        ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
    }
    ImGui::PushStyleVar( ImGuiStyleVar_ButtonTextAlign, ImVec2( 0.0f, 0.5f ) );

    std::string str = std::format( "{} {}", icon, label );
    if ( collapsed ) str = icon;
    bool clicked = ImGui::Button( str.c_str( ), width );

    ImGui::PopStyleVar( );
    if ( active ) ImGui::PopStyleColor( 3 );
    return clicked;
}
