#pragma once
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

inline bool configExists() {
    std::string home = std::getenv("HOME");
    std::string backupPath = home + "/.config/savemanager/backup";

    if(!fs::exists(backupPath)) {
        return fs::create_directories(backupPath);
    }
   
    return true;
}
