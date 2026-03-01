#include "config.hpp"
#include "core/logger/logger.hpp"
#include <filesystem>

static logger configLog;

bool Config::config_exist() {
    if(!fs::exists(backup_dir) || !fs::exists(cache_dir)) {
        if(!fs::create_directories(backup_dir)) {
            configLog.error("Failed to create backup directory");
            return false;
        }
        if(!fs::create_directories(cache_dir)) {
            configLog.error("Failed to create cache directory");
            return false;
        }
    }

    fs::path json_file = config_dir / "gameids.json";
    if(!fs::exists(json_file)) {
        configLog.info("gameids.json not found, downloading...");
        if(!download_ubi_translations()) {
            configLog.error("Failed to download Ubisoft game IDs");
            return false;
        }
    }

    return true;
}
