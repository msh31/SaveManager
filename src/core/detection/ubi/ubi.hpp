#pragma once
#include "core/detection/detection.hpp"
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace ubi {
    void find_saves(const fs::path& prefix, std::vector<Game>& out_games, std::string& out_uuid);
}
