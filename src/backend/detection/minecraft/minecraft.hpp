#pragma once
#include <types.hpp>

class MinecraftDetector {
public:
    std::expected<std::vector<Game>, DetectionError> find_saves() const;

private:
    std::vector<Game> scan_official() const;
    std::vector<Game> scan_modrinth() const;
    std::vector<Game> scan_curseforge() const;
    std::vector<Game> scan_prism() const;
    std::vector<Game> scan_multimc() const;
    std::vector<Game> scan_atllauncher() const;
};
