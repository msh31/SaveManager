#pragma once
#include "../idetector.hpp"

class CWinePrefixDetector : public IDetector {
    public:
        CWinePrefixDetector( fs::path prefix );

        std::expected<std::vector<Game>, SMError> find( ) override;

        std::string_view name( ) const override;

    private:
        fs::path m_path = { }; // must be set
};
