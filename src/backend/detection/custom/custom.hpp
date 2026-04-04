#pragma once
#include "backend/detection/idetector.hpp"
#include "backend/utils/custom_games/custom_games.hpp"

class CustomDetector : public IDetector {
public:
    void find_saves(const fs::path& prefix, std::vector<Game>& out_games) const override;
private:
    const std::vector<CustomGamesFile::CustomGame> default_games = {
        {"Crimson Desert", "AppData/Local/Pearl Abyss/CD/save", "3321460"},
    };
};
