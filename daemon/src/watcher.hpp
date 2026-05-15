#pragma once
#include <chrono>
#include <thread>
#include <map>
#include <config/config.hpp>

class Watcher {
    public:
        Watcher(std::function<void(const fs::path&, uint32_t)> fun, const Config& config);
        ~Watcher() {
            shutdown();
        }

        bool add_watch(const fs::path& path);
        bool remove_watch(const fs::path& path);

        void run();
    private:
        Config config;
        std::function<void(const fs::path&, uint32_t)> m_fun;

        int m_notify_fd = -1;
        int m_signal_fd = -1;

        std::map<fs::path, int> m_watch_descriptors;
        std::map<int, fs::path> m_wd_to_path;

        std::atomic<bool> m_running;
        std::map<std::filesystem::path, std::pair<std::chrono::system_clock::time_point, uint32_t>> save_event_times;
        std::mutex debounce_mutex;
        std::thread m_debounce_thread;
        void debounce_loop();
        std::chrono::milliseconds interval{500};

        void shutdown();
};
