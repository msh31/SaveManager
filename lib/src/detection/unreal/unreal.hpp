#pragma once
#include "../idetector.hpp"
#include <utils/translations/translations.hpp>

class CUnrealDetector : public IDetector {
    public:
        CUnrealDetector( const Translations& translations ) : m_translations( translations ) {}
        std::expected<std::vector<Game>, SMError> find( ) override;

        std::string_view name( ) const override;

    private:
        friend class CWinePrefixDetector;

        static std::vector<Game> scan( fs::path, const Translations& translations );
        const Translations& m_translations;

        // uses the gvas header to identify saves
        static std::vector<Game> scan_recursive( const fs::path& path, const Translations& translations );
        static std::optional<fs::path> resolve_save_games( const fs::path& folder ); // windows only
};
