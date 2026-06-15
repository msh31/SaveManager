#pragma once
#include <backend/task_runner/task_runner.hpp>
#include <utils/paths.hpp>

#include <frontend/layout/tabbar/tabbar.hpp>

#include <backend/shader/shader.hpp>
#include <backend/ui_manager/ui_manager.hpp>
#include <config/config.hpp>

class CApp {
    public:
        ~CApp( );
        void init( );
        void render( );
        void render_shader( std::pair<int, int> window_size );

    private:
        CConfig m_config;

        CUIManager m_ui_manager{ std::make_unique<CTabbarShell>( ) };

        CTaskRunner m_task_runner;
        std::optional<CShader> m_shader;
};
