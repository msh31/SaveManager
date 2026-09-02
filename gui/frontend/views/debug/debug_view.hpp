#pragma once
#include <frontend/views/base_view.hpp>
#include <async_queue/async_queue.hpp>

class CDebugView : public CBaseView {
    public:
        ~CDebugView( ) override;
        void render( ) override;
        void on_exit( ) override;

        void set_dropped_paths( const std::vector<std::string>& files );

    private:
        CAsyncQueue m_queue;
        std::optional<TaskHandle> m_task_handle;

        std::ifstream m_file;
        std::string m_file_path;
        std::string m_file_size; // shown with the proper uh suffix like GB, KB etc..
        std::string m_file_name;
        bool m_is_file_open = false;

        std::vector<std::string> m_dropped_paths;
};
