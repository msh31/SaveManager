#include "config.hpp"
#include <filesystem>

bool Config::config_exist() {
    if(!fs::exists(backup_dir) || !fs::exists(cache_dir)) {
        if(!fs::create_directories(backup_dir)) {
            return false;
        }
        if(!fs::create_directories(cache_dir)) {
            return false;
        }
    }

    fs::path json_file = config_dir / "gameids.json";
    if(!fs::exists(json_file)) {
        //to be truely local, this should change in the future
        if(!download_ubi_translations()) {
            return false;
        }
    }

    return true;
}
