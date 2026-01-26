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
        std::string uuid;
        std::string game_id;
        std::string game_name;
        fs::path save_path;
    };

    static std::vector<fs::path> getLibraryFolders();
    static std::vector<UbiGame> findSaves();
private:
    static std::optional<fs::path> getSteamLocation();
};
