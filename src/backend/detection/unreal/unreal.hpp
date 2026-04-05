#pragma once
#include "backend/detection/idetector.hpp"

class UnrealDetector {
public:
    enum class ScanMode { Recursive, Native };
    void find_saves(const fs::path& prefix, std::vector<Game>& out_games, ScanMode mode = ScanMode::Recursive) const;
private:
    char header[4] = {'G','V','A','S'};
    void scan_for_saves(const fs::path& path, std::set<fs::path>& directories) const;

    const std::unordered_map<std::string_view, std::string> translations = {
        {"MGSDelta", "2417610"},
        {"detnoir", "1939970"},
        {"Sandfall", "1903340"},
        {"Sackboy", "1599660"},
        {"ReadyOrNot", "1144200"},
        {"Oregon", "1583230"},
        {"Ghost", "2479650"},
        {"Maneater", "629820"},
        {"Mafia The Old Country", "1941540"},
    };
};
