#pragma once
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

inline bool config_exists() {
    std::string home = std::getenv("HOME");
    std::string backupPath = home + "/.config/savemanager/backup";

    if(!fs::exists(backupPath)) {
        return fs::create_directories(backupPath);
    }
   
    return true;
}
