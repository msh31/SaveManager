#pragma once
#include "core/detection/idetector.hpp"

class UbisoftDetector : public IDetector {
public:
    void find_saves(const fs::path& prefix, std::vector<Game>& out_games) const override;
    void find_anno_saves(const fs::path& prefix, std::vector<Game>& out_games) const;

private:
    struct AnnoEntry {
        std::string game_name;
        std::string save_subpath;
    };

    const std::unordered_map<std::string_view, AnnoEntry> anno_paths = { 
        {"Anno 117 - Pax Romana", {"Anno 117: Pax Romana", "accounts"}},
        {"Anno 1800", {"Anno 1800", "accounts"}},
        {"Anno 1404",             {"Anno 1404", "Savegames"}},
        {"Anno1404",             {"Anno 1404", "Profiles"}},
        {"Anno 1404 Venice",             {"Anno 1404 Venice", "Savegames"}},
        {"Anno 1503 History Edition",             {"Anno 1503 History Edition", "SaveGame"}},
        {"Anno 1602 History Edition",             {"Anno 1602 History Edition", ""}},
        {"Anno 1701 History Edition",             {"Anno 1701 History Edition", ""}},
        {"Anno 2070",             {"Anno 2070", "Accounts"}},
    };
};
