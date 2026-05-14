#include "watcher.hpp"

#if defined(__linux__)
#include <sys/inotify.h>
#endif

#include <utils/paths.hpp>
#include <config/config.hpp>

int main() {
    Config config;
#if defined(__linux__)
    Watcher watcher([](const fs::path& path, uint32_t mask) { 
            if (mask & IN_CREATE)       SPDLOG_INFO("created: {}", path.string());
            else if (mask & IN_DELETE)  SPDLOG_INFO("deleted: {}", path.string());
            else if (mask & IN_CLOSE_WRITE) SPDLOG_INFO("modified: {}", path.string());
            });
    for (auto& entry : config.settings.watch_paths) {
        if(!watcher.add_watch(entry)) {
            SPDLOG_ERROR("failed to add watcher for: {}", entry.string());
        }
    }
    watcher.run();
#else
    return 0;
#endif
}
