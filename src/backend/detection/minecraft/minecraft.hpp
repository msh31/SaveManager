#pragma once
#include <types.hpp>

class MinecraftDetector {
public:
    std::expected<std::vector<Game>, DetectionError> find_saves() const;

private:
    //To be expanded
    enum LAUNCHER_TYPE {
        OFFICIAL = 1,
        MODRINTH,
        CURSEFORGE,
        PRISM,
        MULTIMC,
        ATLLAUNCHER
    };

    std::vector<Game> scan_official() const;
    std::vector<Game> scan_modrinth() const;
    std::vector<Game> scan_curseforge() const;
    std::vector<Game> scan_prism() const;
    std::vector<Game> scan_multimc() const;
    std::vector<Game> scan_atllauncher() const;
};
