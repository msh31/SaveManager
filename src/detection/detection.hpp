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

    static std::vector<fs::path> getLibraryFolders();
    static DetectionResult findSaves();
private:
    static std::optional<fs::path> getSteamLocation();
};
