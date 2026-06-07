#pragma once
#include <types.hpp>

class CMinecraftDetector {
    public:
        std::expected<std::vector<Game>, SMError> find_saves( ) const;

    private:
        std::vector<Game> scan_official( ) const;
        std::vector<Game> scan_modrinth( ) const;
        std::vector<Game> scan_curseforge( ) const;
        std::vector<Game> scan_prism( ) const;
        std::vector<Game> scan_multimc( ) const;
        // std::vector<Game> scan_atllauncher() const;
};
