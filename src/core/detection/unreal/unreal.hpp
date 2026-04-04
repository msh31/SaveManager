#pragma once
#include "core/detection/idetector.hpp"

class UnrealDetector : public IDetector {
public:
    void find_saves(const fs::path& prefix, std::vector<Game>& out_games) const override;
private:
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
