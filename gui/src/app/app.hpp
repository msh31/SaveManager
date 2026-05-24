#pragma once
#include <backend/config/config.hpp>
#include <backend/ui_manager/ui_manager.hpp>
#include <utils/paths.hpp>

// #include <frontend/layout/tabbar/tabbar.hpp>
#include <frontend/layout/sidebar/sidebar.hpp>
// #include <frontend/layout/ribbon/ribbon.hpp>

class CApp {
    public:
    void init( );
    void render( );

    private:
    CConfig m_config;
    // CUIManager m_ui_manager { std::make_unique<CTabbarShell>() };
    CUIManager m_ui_manager{ std::make_unique<CSidebarShell>( ) };
    // CUIManager m_ui_manager { std::make_unique<CRibbonShell>() };

    bool m_toggle_a = false, m_toggle_b = false, m_toggle_c = true, m_toggle_d = false, m_toggle_e = false;

    CMenuBar m_menubar;
    CStatusBar m_statusbar;
};
