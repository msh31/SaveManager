#pragma once
#include <backend/image_manager/image_manager.hpp>
#include <backend/ui_manager/ui_manager.hpp>
#include <async_queue/async_queue.hpp>

#include <config/config.hpp>
#include <utils/paths.hpp>

#include <frontend/layout/tabbar/tabbar.hpp>
// #include <frontend/layout/sidebar/sidebar.hpp>
//  #include <frontend/layout/ribbon/ribbon.hpp>
//  #include <frontend/layout/none/none.hpp>

class CDebugView;

class CApp {
    public:
        explicit CApp( CConfig& config ) : m_config( config ) {}

        void init( );
        void render( );
        void on_files_dropped( const std::vector<std::string>& );

    private:
        void refresh_background( );

        CConfig& m_config;
        CAsyncQueue m_queue;
        std::optional<TaskHandle> m_task_handle;
        CDebugView* m_debug_view = nullptr;

        CUIManager m_ui_manager{ std::make_unique<CTabbarShell>( ) };
        // CUIManager m_ui_manager{ std::make_unique<CSidebarShell>( ) };
        //  CUIManager m_ui_manager{ std::make_unique<CNoShell>( ) }; // kinda sucks
        //  CUIManager m_ui_manager { std::make_unique<CRibbonShell>() };

        CMenuBar m_menubar;
        CStatusBar m_statusbar;

        ImageData m_background_image = { };
        std::string m_loaded_bg_name = { };
};
