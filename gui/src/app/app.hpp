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
};
