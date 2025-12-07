#pragma once
#include <string>
#include <vector>
#include <chrono>
#include <filesystem>

#include <sentinel/core/sentinel.h>
#include <json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;

enum class PlatformTypes {
    Ubisoft,
    Rockstar,
    Epic,
};

enum class BackupStatus {
    None,
    Partial,
    Complete
};

struct GameInfo {
    std::string gameName;
    std::string savePath;
    std::string documentFolder;
    
    PlatformTypes platform;
    
    int gameID;
    int saveCount;

    fs::file_time_type lastModified;
};

struct BackupRecord {
    int gameID;
    int filesBackedUp;
    BackupStatus status;
    std::chrono::system_clock::time_point backupDate;
    std::string backupPath;
};

#include "detection/profile.hpp"
#include "detection/games.hpp"
