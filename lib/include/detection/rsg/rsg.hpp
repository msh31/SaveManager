#pragma once
#include <detection/idetector.hpp>

class CRockstarDetector : public IDetector {
    public:
        std::expected<std::vector<Game>, SMError> find( ) override;

        std::string_view name( ) const override;

    private:
        const std::unordered_map<std::string_view, std::string> m_legacy_games = {
            // small enough, for now.
            { "GTA3 User Files", "Grand Theft Auto III" },
            { "GTA Vice City User Files", "Grand Theft Auto Vice City" },
            { "GTA San Andreas User Files", "Grand Theft Auto San Andreas" },
            { "Manhunt User Files", "Manhunt" },
            { "Manhunt 2", "Manhunt 2" },
        };
};
