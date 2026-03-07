#pragma once
#include "core/detection/detection.hpp"
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

namespace rsg {
void find_saves(const fs::path& prefix, std::vector<Game>& out_games);
}
