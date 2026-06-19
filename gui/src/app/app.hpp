#pragma once
#include <config/config.hpp>

#include <backend/shader/shader.hpp>
#include <backend/task_runner/task_runner.hpp>
#include <backend/ui_manager/ui_manager.hpp>

#include <utils/blacklist/blacklist.hpp>
#include <utils/paths.hpp>

#include <frontend/layout/tabbar/tabbar.hpp>

class CApp {
    public:
        CApp( Blacklist& blacklist ) : m_blacklist( blacklist ) {};
        ~CApp( );
        void init( );
        void render( );
        void render_shader( std::pair<int, int> window_size );

    private:
        CConfig m_config;

        CUIManager m_ui_manager{ std::make_unique<CTabbarShell>( ) };
        Blacklist& m_blacklist; // injected

        CTaskRunner m_task_runner;
        std::optional<CShader> m_shader;
};
