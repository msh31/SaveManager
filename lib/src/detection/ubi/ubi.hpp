#pragma once
#include "../detector_context.hpp"
#include "../idetector.hpp"
#include <utils/translations/translations.hpp>

class CUbisoftDetector : public IDetector {
    public:
        static constexpr std::string_view PLATFORM_LABEL = "Ubisoft";

        CUbisoftDetector( const Translations& translations ) : m_translations( translations ) {}

        std::expected<std::vector<Game>, SMError> find( ) override;

        std::string_view name( ) const override;

        static std::vector<Game> scan_wine_user( const fs::path& user_home, const DetectorContext& ctx );
        static std::vector<Game> scan_wine_prefix( const fs::path& drive_c, const DetectorContext& ctx );

    private:
        static std::vector<Game> scan( fs::path, const Translations& );
        const Translations& m_translations;

        struct AnnoEntry {
                std::string game_name;
                std::string save_subpath;
        };
};
