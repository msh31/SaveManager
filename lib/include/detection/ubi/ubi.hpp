#pragma once
#include <detection/idetector.hpp>

class CUbisoftDetector : public IDetector {
    public:
        std::expected<std::vector<Game>, SMError> find( ) override;

        std::string_view name( ) const override;

    private:
        friend class CWinePrefixDetector;
        static std::vector<Game> scan( fs::path );

        struct AnnoEntry {
                std::string game_name;
                std::string save_subpath;
        };
};
