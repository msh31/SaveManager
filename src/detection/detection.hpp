#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include <optional>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

class Detection {
public:
    struct UbiGame {
        std::string appid;
        std::string game_id;
        std::string game_name;
        fs::path save_path;
    };

    struct DetectionResult {
        std::string uuid;
        std::vector<UbiGame> games;
    };

    static std::vector<fs::path> get_library_folders();
    static DetectionResult find_saves();

private:
    static std::vector<std::string> get_platform_steam_paths();
    static std::optional<fs::path> get_steam_location();
};
