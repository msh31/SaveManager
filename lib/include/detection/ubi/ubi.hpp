#pragma once
#include <detection/idetector.hpp>

class CUbisoftDetector : public IDetector {
    public:
        std::expected<std::vector<Game>, SMError> find( ) override;

        std::string_view name( ) const override;

    private:
        struct AnnoEntry {
                std::string game_name;
                std::string save_subpath;
        };

        const std::unordered_map<std::string_view, AnnoEntry> m_anno_paths = {
            { "Anno 117 - Pax Romana", { "Anno 117: Pax Romana", "accounts" } },
            { "Anno 1800", { "Anno 1800", "accounts" } },
            { "Anno 1404", { "Anno 1404", "Savegames" } },
            { "Anno1404", { "Anno 1404", "Profiles" } },
            { "Anno 1404 Venice", { "Anno 1404 Venice", "Savegames" } },
            { "Anno 1503 History Edition", { "Anno 1503 History Edition", "SaveGame" } },
            { "Anno 1602 History Edition", { "Anno 1602 History Edition", "" } },
            { "Anno 1701 History Edition", { "Anno 1701 History Edition", "" } },
            { "Anno 2070", { "Anno 2070", "Accounts" } },
        };
};
