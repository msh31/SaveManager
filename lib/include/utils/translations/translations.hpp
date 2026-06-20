#pragma once
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct Translations {
        bool init( );
        std::optional<std::string> get_game_name_rsg( std::string_view folder_name ) const;
        std::optional<std::string> get_game_name_ubi( const std::string& game_id ) const;
        std::optional<std::string> get_steam_id( const std::string& game_name ) const;
        std::optional<std::string> get_steam_name( const std::string& appid ) const;

    private:
        std::unordered_map<std::string, std::string> m_steam_n2i_translation = { }; // name to appid
        std::unordered_map<std::string, std::string> m_steam_i2n_translation = { }; // appid to name

        std::unordered_map<std::string, std::string> m_ubi_translations = { };
};
