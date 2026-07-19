#pragma once
#include "../detector_context.hpp"
#include "../idetector.hpp"
#include <utils/translations/translations.hpp>

class CRockstarDetector : public IDetector {
    public:
        static constexpr std::string_view PLATFORM_LABEL = "Rockstar";

        CRockstarDetector( const Translations& translations ) : m_translations( translations ) {}

        std::expected<std::vector<Game>, SMError> find( ) override;

        std::string_view name( ) const override;

        static std::vector<Game> scan_wine_user( const fs::path& user_home, const DetectorContext& ctx );

    private:
        static std::vector<Game> scan( fs::path, const Translations& translations );
        const Translations& m_translations;
};
