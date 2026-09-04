#pragma once
#include <backend/image_manager/image_manager.hpp>
#include <backend/ui_manager/ui_manager.hpp>
#include <async_queue/async_queue.hpp>

#include <frontend/layout/tabbar/tabbar.hpp>

class CDebugView;

class CApp {
    public:

        void init( );
        void render( );
        void on_files_dropped( const std::vector<std::string>& );

    private:
        void refresh_background( );

        CAsyncQueue m_queue;
        std::optional<TaskHandle> m_update_handle;

        CUIManager m_ui_manager{ std::make_unique<CTabbarShell>( ) };

        ImageData m_background_image = { };
        std::string m_loaded_bg_name = { };

        std::optional<TaskHandle> m_detection_handle;
};
