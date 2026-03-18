#pragma once
#include <filesystem>
#include <vector>
#include <optional>

namespace fs = std::filesystem;

class Config;

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
        std::vector<Game> games;
    };

    DetectionResult find_saves(Config& config);
};
