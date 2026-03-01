#include "config.hpp"
#include "core/helpers/network.hpp"
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

    fs::path ubi_translations = config_dir / "ubi_translations.json";
    fs::path steam_appids = config_dir / "steamids.json";

    if(!fs::exists(ubi_translations)) {
        configLog.info("ubi_translations.json not found, downloading...");
        if(!download_file("https://git.marco007.dev/marco/smdata/raw/branch/main/ubi_translations.json", ubi_translations)) {
            configLog.error("Failed to download Ubisoft game IDs");
            return false;
        }
    }
    if(!fs::exists(steam_appids)) {
        configLog.info("steamids.json was not found, downloading...");
        if(!download_file("https://git.marco007.dev/marco/smdata/raw/branch/main/steamids.json", steam_appids)) {
            configLog.error("Failed to download Steam IDs");
            return false;
        }
    }

    return true;
}
