#pragma once
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct Translations {
        bool init( );
        std::optional<std::string> get_game_name_rsg( std::string_view folder_name ) const;
        std::optional<std::string> get_game_name_ubi( const std::string& game_id ) const;

    private:
        std::unordered_map<std::string, std::string> m_ubi_translations = { };

        mutable std::mutex m_translations_mutex;
};
