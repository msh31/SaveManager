#pragma once
#include "backend/detection/detection.hpp"
#include "backend/utils/custom_games/custom_games.hpp"

class CustomDetector {
public:
    std::expected<std::vector<Game>, DetectionError> find_saves(const fs::path& prefix) const;
private:
    const std::vector<CustomGamesFile::CustomGame> default_games = {
        {"Crimson Desert", "AppData/Local/Pearl Abyss/CD/save", "3321460"},
    };
};
