#include "watcher.hpp"
#include <utils/paths.hpp>

int main() {
    Watcher watcher([](const fs::path& path) { SPDLOG_INFO("modified: {}", path.string()); });
    if(!watcher.add_watch(paths::plugin_dir())) {
        SPDLOG_ERROR("failed to add watcher for: {}", paths::plugin_dir().string());
    }
    watcher.run();
}
