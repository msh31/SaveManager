#pragma once
#include "../idetector.hpp"
#include <utils/translations/translations.hpp>

class CWinePrefixDetector : public IDetector {
    public:
        CWinePrefixDetector( fs::path prefix, const Translations& translations )
            : m_path( std::move( prefix ) ), m_translations( translations ) {};

        std::expected<std::vector<Game>, SMError> find( ) override;

        std::string_view name( ) const override;

    private:
        fs::path m_path = { }; // must be set
        const Translations& m_translations;
};
