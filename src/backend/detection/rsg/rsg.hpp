#pragma once
#include "backend/detection/idetector.hpp"

class RockstarDetector : public IDetector {
public:
    void find_saves(const fs::path& prefix, std::vector<Game>& out_games) const override;
    void find_legacy_saves(const fs::path& prefix, std::vector<Game>& out_games) const;

private:
    const std::unordered_map<std::string_view, std::string> legacy_games = { //small enough, for now.
        {"GTA3 User Files", "Grand Theft Auto III"},
        {"GTA Vice City User Files", "Grand Theft Auto Vice City"},
        {"GTA San Andreas User Files", "Grand Theft Auto San Andreas"},
        {"Manhunt User Files", "Manhunt"},
        {"Manhunt 2", "Manhunt 2"},
    };
};
