#pragma once
#include "core/detection/idetector.hpp"
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

class RockstarDetector : public IDetector {
public:
    void find_saves(const fs::path& prefix, std::vector<Game>& out_games) const override;
};
