#pragma once
#include <config/config.hpp>

#include <backend/shader/shader.hpp>
#include <backend/task_runner/task_runner.hpp>
#include <backend/ui_manager/ui_manager.hpp>
#include <utils/paths.hpp>

#include <utils/blacklist/blacklist.hpp>
#include <utils/translations/translations.hpp>

#include <frontend/layout/tabbar/tabbar.hpp>

class CApp {
    public:
        ~CApp( );
        void init( );
        void render( );
        void render_shader( std::pair<int, int> window_size );

        const CConfig::WindowProperties& window_props( ) const { return m_config.win_props; }
        void save_window_props( int x, int y, int width, int height );

    private:
        CConfig m_config;

        CUIManager m_ui_manager{ std::make_unique<CTabbarShell>( ) };
        Blacklist m_blacklist;
        Translations m_translations;

        CTaskRunner m_task_runner;
        std::optional<CShader> m_shader;
};
