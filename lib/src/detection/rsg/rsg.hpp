#pragma once
#include "../idetector.hpp"

class CRockstarDetector : public IDetector {
    public:
        std::expected<std::vector<Game>, SMError> find( ) override;

        std::string_view name( ) const override;

    private:
        friend class CWinePrefixDetector; // a friend wants to use this function
        static std::vector<Game> scan( fs::path );
};
