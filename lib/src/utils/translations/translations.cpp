#include "utils/translations/translations.hpp"
#include "utils/paths.hpp"

static const std::unordered_map<std::string_view, std::string> rsg_translations = {
    { "GTA III", "Grand Theft Auto III" },
    { "GTA Vice City", "Grand Theft Auto: Vice City" },
    { "GTA San Andreas", "Grand Theft Auto: San Andreas" },
    { "Bully", "Bully: Scholarship Edition" },
    { "GTA IV", "Grand Theft Auto IV: The Complete Edition" },
    { "GTA V", "Grand Theft Auto V Legacy" },
    { "GTA San Andreas Definitive Edition", "Grand Theft Auto: San Andreas – The Definitive Edition" },
    { "GTA Vice City Definitive Edition", "Grand Theft Auto: Vice City – The Definitive Edition" },
    { "GTA III Definitive Edition", "Grand Theft Auto III – The Definitive Edition" },
    { "GTAV Enhanced", "Grand Theft Auto V Enhanced" },
};

bool Translations::init( ) {
    json ubi_data, steam_data;
    std::ifstream ubi_file( paths::ubi_translations( ).c_str( ) );
    std::ifstream steam_file( paths::steam_appids( ).c_str( ) );

    try {
        if ( ubi_file.is_open( ) && fs::file_size( paths::ubi_translations( ) ) > 0 ) {
            ubi_data = json::parse( ubi_file );
            SPDLOG_INFO( "Loaded Ubisoft translations with {} franchises", ubi_data.size( ) );
            for ( const auto& [franchise, games] : ubi_data.items( ) ) {
                for ( const auto& [id, name] : games.items( ) ) {
                    auto clean = std::regex_replace( name.get<std::string>( ), std::regex( R"(\s*\([^)]*\))" ), "" );
                    m_ubi_translations[id] = clean;
                }
            }
        } else {
            SPDLOG_ERROR( "Failed to open ubisoft translations file!" );
            return false;
        }
    } catch ( json::exception& ex ) {
        SPDLOG_ERROR( "Translation error: {}", ex.what( ) );
        return false;
    }

    try {
        if ( steam_file.is_open( ) && fs::file_size( paths::steam_appids( ) ) > 0 ) {
            steam_data = json::parse( steam_file );
            SPDLOG_INFO( "Loaded steamid translations" );
            for ( const auto& [platform, games] : steam_data.items( ) ) {
                for ( const auto& game : games ) {
                    std::string name = game["name"].get<std::string>( );
                    std::string appid = std::to_string( game["appid"].get<int>( ) );
                    m_steam_n2i_translation[name] = appid;
                    m_steam_i2n_translation[appid] = name;
                }
            }
        } else {
            SPDLOG_ERROR( "Failed to open steamids file!" );
            return false;
        }
    } catch ( json::exception& ex ) {
        SPDLOG_ERROR( "Translation error: {}", ex.what( ) );
        return false;
    }
    return true;
}

std::optional<std::string> Translations::get_game_name_ubi( const std::string& game_id ) const {
    std::lock_guard<std::mutex> lock( m_translations_mutex );
    if ( auto it = m_ubi_translations.find( game_id ); it != m_ubi_translations.end( ) ) {
        return it->second;
    }
    return std::nullopt;
}

std::optional<std::string> Translations::get_steam_id( const std::string& game_name ) const {
    std::lock_guard<std::mutex> lock( m_translations_mutex );
    if ( auto it = m_steam_n2i_translation.find( game_name ); it != m_steam_n2i_translation.end( ) ) {
        return it->second;
    }

    return std::nullopt;
}

std::optional<std::string> Translations::get_steam_name( const std::string& appid ) const {
    std::lock_guard<std::mutex> lock( m_translations_mutex );
    if ( auto it = m_steam_i2n_translation.find( appid ); it != m_steam_i2n_translation.end( ) ) {
        return it->second;
    }

    return std::nullopt;
}

std::optional<std::string> Translations::get_game_name_rsg( std::string_view folder_name ) const {
    std::lock_guard<std::mutex> lock( m_translations_mutex );
    if ( auto it = rsg_translations.find( folder_name ); it != rsg_translations.end( ) ) {
        return it->second;
    }
    return std::nullopt;
}
