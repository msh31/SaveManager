#include "watcher.hpp"

#if defined(__linux__)
#include <sys/inotify.h>
#endif

#include <constants.hpp>
#include <utils/paths.hpp>
#include <config/config.hpp>
#include <logger/logger.hpp>

#include <features/scheduler/scheduler.hpp>

int main() {
    Config config;

    SaveScheduler scheduler(config);
    ScheduleEntry test = {
        true,
        "243470",
        "Watch_Dogs",
        "/mnt/data/SteamLibrary/steamapps/compatdata/243470/pfx/drive_c/Program Files (x86)/Ubisoft/Ubisoft Game Launcher/savegames/185654f7-5a7b-44de-baa7-76c4f26fbda4/541/",
        1,
        -1,
    };
    scheduler.add_entry(test);

    spdlog::set_level(spdlog::level::debug);
    init_logger("[%n]: [%l] %d-%m-%Y %H:%M:%S - %v", "savemanager-daemon");
    SPDLOG_INFO("daemon started!");

#if defined(__linux__)
    Watcher watcher([](const fs::path& path, uint32_t mask) { 
            if(!fs::exists(path)) SPDLOG_WARN("{} does not exist", path.string());
            auto ext = path.extension().string();
            if (std::ranges::contains(extension_blocklist, ext) || 
                std::ranges::contains(g_extension_blocklist, ext)) return;

            if (mask & IN_CREATE)       SPDLOG_INFO("created: {}", path.string());
            else if (mask & IN_DELETE)  SPDLOG_INFO("deleted: {}", path.string());
            else if (mask & IN_CLOSE_WRITE) SPDLOG_INFO("modified: {}", path.string());
            }, config);

    for (auto& entry : config.settings.watch_paths) {
        if(!watcher.add_watch(entry)) {
            SPDLOG_ERROR("failed to add watcher for: {}", entry.string());
        }
    }
    scheduler.run();
    watcher.run();
#else
    return 0;
#endif
}
