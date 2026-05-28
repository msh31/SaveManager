#pragma once
#include <backend/shader/shader.hpp>
#include <backend/ui_manager/ui_manager.hpp>
#include <config/config.hpp>
#include <utils/paths.hpp>

#include <frontend/layout/tabbar/tabbar.hpp>

class CApp {
    public:
        void init( );
        void render( );
        void render_shader( std::pair<int, int> window_size );

    private:
        CConfig                m_config;
        CUIManager             m_ui_manager{ std::make_unique<CTabbarShell>( ) };
        std::optional<CShader> m_shader;
};
