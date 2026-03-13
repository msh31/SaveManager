#pragma once
#include "core/detection/detection.hpp"
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

namespace unreal {
void find_saves(const fs::path& prefix, std::vector<Game>& out_games);
}
