#pragma once
#include <detection/idetector.hpp>

class CMinecraftDetector : public IDetector {
    public:
        std::expected<std::vector<Game>, SMError> find( ) override;

        std::string_view name( ) const override;

    private:
        friend class CWinePrefixDetector;
        static std::vector<Game> scan( fs::path );

        std::vector<Game> scan_official( ) const;
        std::vector<Game> scan_modrinth( ) const;
        std::vector<Game> scan_curseforge( ) const;
        std::vector<Game> scan_prism( ) const;
        std::vector<Game> scan_multimc( ) const;
};
