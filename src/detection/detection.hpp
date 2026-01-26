#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include <optional>
#include <fstream>

namespace fs = std::filesystem;

class Detection {
public:
    static std::vector<fs::path> getLibraryFolders();
private:
    static std::optional<fs::path> getSteamLocation();
};
