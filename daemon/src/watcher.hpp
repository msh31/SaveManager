#pragma once

class Watcher {
    public:
        Watcher(std::function<void(const fs::path&)> fun);
        ~Watcher() {
            shutdown();
        }

        bool add_watch(const fs::path& path);
        bool remove_watch(const fs::path& path);

        void run();
    private:
        std::function<void(const fs::path&)> m_fun;

        int m_notify_fd = -1;
        int m_signal_fd = -1;

        std::map<fs::path, int> m_watch_descriptors;
        std::map<int, fs::path> m_wd_to_path;

        void shutdown();
};
