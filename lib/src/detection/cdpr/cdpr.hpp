#pragma once
#include "../detector_context.hpp"
#include "../idetector.hpp"

class CCDPRDetector : public IDetector {
    public:
        static constexpr std::string_view PLATFORM_LABEL = "CD Projekt Red";

        std::expected<std::vector<Game>, SMError> find( ) override;

        std::string_view name( ) const override;

        static std::vector<Game> scan_wine_user( const fs::path& user_home, const DetectorContext& ctx );
};
