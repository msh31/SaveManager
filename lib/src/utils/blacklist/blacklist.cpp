#include "utils/blacklist/blacklist.hpp"
#include "utils/paths.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void Blacklist::init( ) {
    json data;
    std::ifstream file( paths::blacklist( ).string( ).c_str( ) );

    if ( file.is_open( ) ) {
        try {
            data = json::parse( file );
            SPDLOG_INFO( "Loaded blacklist JSON" );

            for ( const auto &entry : data ) {
                blacklisted_games.insert( entry.get<std::string>( ) );
            }
        } catch ( json::exception &ex ) {
            SPDLOG_ERROR( "blacklist parsing error: {}", ex.what( ) );
        }
    } else {
        SPDLOG_ERROR( "Failed to open blacklist to load it!" );
    }
}

void Blacklist::save( ) {
    json data;
    for ( const auto &entry : blacklisted_games ) {
        data.emplace_back( entry );
    }

    std::ofstream file( paths::blacklist( ).string( ).c_str( ) );
    file << data.dump( 4 );
}

bool Blacklist::is_blacklisted( const std::string &game_name ) { return blacklisted_games.count( game_name ) > 0; }
