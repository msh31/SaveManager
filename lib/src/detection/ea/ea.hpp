#pragma once
#include "../idetector.hpp"
#include <utils/translations/translations.hpp>

class CElectronicArtsDetector : public IDetector {
    public:
        CElectronicArtsDetector( const Translations& translations ) : m_translations( translations ) {}

        std::expected<std::vector<Game>, SMError> find( ) override;

        std::string_view name( ) const override;

    private:
        friend class CWinePrefixDetector; // a friend wants to use this function

        static std::vector<Game> scan( fs::path, const Translations& translations );
        const Translations& m_translations;
};
