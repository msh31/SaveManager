#pragma once
#include <string>
#include <vector>
#include <chrono>
#include <filesystem>

#include <sentinel/core/sentinel.h>
#include <json.hpp>

#include "detection/profile.hpp"

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

    PlatformTypes platform;

    int gameID;
    int saveCount;

    std::chrono::system_clock::time_point lastModified;
};

struct BackupRecord {
    int gameID; //linked to gameinfo's gameid
    int filesBackedUp;
    BackupStatus status;
    std::chrono::system_clock::time_point backupDate;
    std::string backupPath;
};

class SaveManager {
    public:
        SaveManager();
        ~SaveManager();
};
