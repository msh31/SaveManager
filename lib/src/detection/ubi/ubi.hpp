#pragma once
#include "../idetector.hpp"
#include <utils/translations/translations.hpp>

class CUbisoftDetector : public IDetector {
    public:
        CUbisoftDetector( const Translations& translations ) : m_translations( translations ) {}

        std::expected<std::vector<Game>, SMError> find( ) override;

        std::string_view name( ) const override;

    private:
        friend class CWinePrefixDetector;

        static std::vector<Game> scan( fs::path, const Translations& );
        const Translations& m_translations;

        struct AnnoEntry {
                std::string game_name;
                std::string save_subpath;
        };
};
