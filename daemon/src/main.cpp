#include "watcher.hpp"

#include <utils/paths.hpp>
#include <config/config.hpp>

int main() {
    Config config;
    Watcher watcher([](const fs::path& path) { SPDLOG_INFO("modified: {}", path.string()); });

    for (auto& entry : config.settings.watch_paths) {
        if(!watcher.add_watch(entry)) {
            SPDLOG_ERROR("failed to add watcher for: {}", entry.string());
        }
    }
    watcher.run();
}
