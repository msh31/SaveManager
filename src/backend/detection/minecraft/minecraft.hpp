#pragma once
#include <types.hpp>
namespace fs = std::filesystem;

class MinecraftDetector {
public:
    std::expected<std::vector<Game>, DetectionError> find_saves(const fs::path& prefix) const;

private:
    //To be expanded
    enum LAUNCHER_TYPE {
        OFFICIAL = 1,
        MODRINTH,
        CURSEFORGE,
        BADLION,
        LUNARCLIENT,
    };
};
