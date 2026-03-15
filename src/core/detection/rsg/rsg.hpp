#pragma once
#include "core/detection/idetector.hpp"
#include "core/detection/detection.hpp"
#include "core/helpers/translations/translations.hpp"
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

class RockstarDetector : public IDetector {
public:
    void find_saves(const fs::path& prefix, std::vector<Game>& out_games) const override;
};
