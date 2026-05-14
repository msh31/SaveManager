#include "watcher.hpp"
#include <utils/paths.hpp>

int main() {
    Watcher watcher([](const fs::path& path) { SPDLOG_INFO("modified: {}", path.string()); });
    watcher.add_watch(paths::plugin_dir());
    watcher.run();
}
