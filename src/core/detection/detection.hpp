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

    static DetectionResult find_saves(Config& config);

    static const Game* get_selected_game(const DetectionResult& result);
    DetectionResult find_saves(Config& config);
    std::vector<std::string> get_platform_steam_paths();
    std::optional<fs::path> get_steam_location(Config& config);
    std::vector<fs::path> get_library_folders(Config& config);
};
