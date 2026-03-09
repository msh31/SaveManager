#pragma once
#include "core/config/config.hpp"
#include <filesystem>
#include <string>
#include <vector>
#include <optional>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

enum PlatformType {
    UBISOFT = 1,
    ROCKSTAR,
    UNREAL,
    PSP,
    PPSSPP
};

struct Game {
    PlatformType type;
    std::string appid;
    std::optional<std::string> game_id; 
    std::string game_name;
    fs::path save_path;
};

namespace Detection {
    struct DetectionResult {
        std::string uuid;  // ubi only 
        std::vector<Game> games;
    };

    DetectionResult find_saves(Config& config);
};
