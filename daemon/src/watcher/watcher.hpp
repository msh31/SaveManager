#pragma once
#include <chrono>
#include <config/config.hpp>
#include <map>
#include <thread>

class CWatcher {
    public:
    CWatcher( std::function<void( const fs::path &, uint32_t )> fun, const CConfig &config );
    ~CWatcher( ) { shutdown( ); }

    bool add_watch( const fs::path &path );

    void run( );

    private:
    CConfig m_config;
    std::function<void( const fs::path &, uint32_t )> m_fun;

    int m_notify_fd = -1;
    int m_signal_fd = -1;

    std::map<fs::path, int> m_watch_descriptors;
    std::map<int, fs::path> m_wd_to_path;

    std::atomic<bool> m_running{ false };
    std::map<std::filesystem::path, std::pair<std::chrono::system_clock::time_point, uint32_t>> m_save_event_times;
    std::mutex m_debounce_mutex;
    std::thread m_debounce_thread;
    void debounce_loop( );
    std::chrono::milliseconds m_interval{ 500 };

    void shutdown( );
};
