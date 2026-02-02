#pragma once
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

// for now we assume everything is through steam
struct Game {
    PlatformType type;
    std::string appid;
    std::optional<std::string> game_id; 
    std::string game_name;
    fs::path save_path;
};

class Detection {
public:
    struct DetectionResult {
        std::optional<std::string> uuid;  // ubi only 
        std::vector<Game> games;
    };

    static std::vector<fs::path> get_library_folders();
    static DetectionResult find_saves();

private:
    static std::vector<std::string> get_platform_steam_paths();
    static std::optional<fs::path> get_steam_location();
    DetectionResult search_steam_library(const std::string& pfx_path);

    static DetectionResult find_ubi_saves();
    static DetectionResult find_rsg_saves();
    // static DetectionResult find_ue_saves();
    // static DetectionResult find_psp_saves();
};
