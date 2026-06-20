#include "utils/blacklist/blacklist.hpp"
#include "utils/paths.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

bool Blacklist::init( ) {
    json data;
    std::ifstream file( paths::blacklist( ).string( ).c_str( ) );

    if ( file.is_open( ) ) {
        try {
            data = json::parse( file );
            SPDLOG_INFO( "Loaded blacklist!" );

            for ( const auto& entry : data ) {
                m_blacklisted_games.insert( entry.get<std::string>( ) );
            }
            file.close( );
        } catch ( json::exception& ex ) {
            SPDLOG_ERROR( "blacklist parsing error: {}", ex.what( ) );
            return false;
        }
    } else {
        SPDLOG_ERROR( "Failed to open blacklist to load it!" );
        return false;
    }
    return true;
}

void Blacklist::save( ) {
    json data;
    for ( const auto& entry : m_blacklisted_games ) {
        data.emplace_back( entry );
    }

    std::ofstream file( paths::blacklist( ).string( ).c_str( ) );
    file << data.dump( 4 );
    if ( file.good( ) ) file.close( );
}

bool Blacklist::is_blacklisted( const std::string& game_name ) const {
    return m_blacklisted_games.count( game_name ) > 0;
}

const std::unordered_set<std::string>& Blacklist::games( ) const { return m_blacklisted_games; }

void Blacklist::add( const std::string& name ) {
    m_blacklisted_games.insert( name );
    save( );
}
void Blacklist::remove( const std::string& name ) {
    m_blacklisted_games.erase( name );
    save( );
}
