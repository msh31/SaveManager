#pragma once
#include <backend/ui_manager/ui_manager.hpp>
#include <config/config.hpp>
#include <utils/paths.hpp>

#include <frontend/layout/tabbar/tabbar.hpp>

class CApp {
    public:
        void init( );
        void render( );

    private:
        CConfig    m_config;
        CUIManager m_ui_manager{ std::make_unique<CTabbarShell>( ) };

        bool m_toggle_a = false, m_toggle_b = false, m_toggle_c = true, m_toggle_d = false, m_toggle_e = false;
};
