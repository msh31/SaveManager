#include "ui_manager.hpp"

CUIManager::CUIManager( std::unique_ptr<IShell> shell ) : m_shell( std::move( shell ) ) {}

CBaseView* CUIManager::add_view( ViewConfig cfg ) {
    CBaseView* raw = cfg.view.get( );
    CBaseView::ViewItem item{ cfg.icon, cfg.label, raw };

    m_shell->add_nav_item( item );
    m_views.emplace_back( std::move( cfg.view ) );

    if ( m_active_view == nullptr ) set_active_view( raw );

    return raw;
}

void CUIManager::set_settings_view( ViewConfig cfg ) {
    CBaseView* raw = cfg.view.get( );
    CBaseView::ViewItem item{ cfg.icon, cfg.label, raw };

    m_shell->set_settings( item );
    m_views.emplace_back( std::move( cfg.view ) );
}

void CUIManager::set_active_view( CBaseView* view ) {
    if ( m_active_view ) m_active_view->on_exit( );
    m_active_view = view;
    if ( m_active_view ) m_active_view->on_enter( );
}

void CUIManager::set_menubar( CMenuBar menubar ) { m_menubar = std::move( menubar ); }

void CUIManager::set_statusbar( CStatusBar statusbar ) { m_statusbar = std::move( statusbar ); }

void CUIManager::render( ) {
    if ( m_menubar ) m_menubar->render( );

    float sb_h = m_statusbar ? CStatusBar::height( ) : 0.f;
    ImGui::BeginChild(
        "##shell_area", { 0.f, -sb_h }, ImGuiChildFlags_None,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground );
    if ( auto* clicked = m_shell->render( m_active_view ) ) set_active_view( clicked );
    ImGui::EndChild( );

    if ( m_statusbar ) m_statusbar->render( );
}
