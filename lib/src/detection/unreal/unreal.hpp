#pragma once
#include "../idetector.hpp"

class CUnrealDetector : public IDetector {
    public:
        std::expected<std::vector<Game>, SMError> find( ) override;

        std::string_view name( ) const override;

    private:
        friend class CWinePrefixDetector;
        static std::vector<Game> scan( fs::path );

        // using the gvas header
        static std::vector<Game> scan_recursive( const fs::path& path );
};
