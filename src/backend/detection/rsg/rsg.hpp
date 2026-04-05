#pragma once
#include "backend/detection/detection.hpp"

class RockstarDetector {
public:
    std::expected<std::vector<Game>, DetectionError> find_saves(const fs::path& prefix) const;
    std::expected<std::vector<Game>, DetectionError> find_legacy_saves(const fs::path& prefix) const;

private:
    const std::unordered_map<std::string_view, std::string> legacy_games = { //small enough, for now.
        {"GTA3 User Files", "Grand Theft Auto III"},
        {"GTA Vice City User Files", "Grand Theft Auto Vice City"},
        {"GTA San Andreas User Files", "Grand Theft Auto San Andreas"},
        {"Manhunt User Files", "Manhunt"},
        {"Manhunt 2", "Manhunt 2"},
    };
};
