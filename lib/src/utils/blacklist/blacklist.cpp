#include "blacklist.hpp"
#include "utils/paths.hpp"
#include "utils/utils.hpp"

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

// NOTE: this function does not lock itself and any caller MUST lock the interal mutex
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
    std::lock_guard<std::mutex> lock( m_blacklist_mutex );
    return m_blacklisted_games.count( game_name ) > 0;
}

const std::unordered_set<std::string> Blacklist::games( ) const {
    std::lock_guard<std::mutex> lock( m_blacklist_mutex );
    return m_blacklisted_games;
}

void Blacklist::add( const std::string& name ) {
    std::lock_guard<std::mutex> lock( m_blacklist_mutex );
    m_blacklisted_games.insert( name );
    save( );
}
void Blacklist::remove( const std::string& name ) {
    std::lock_guard<std::mutex> lock( m_blacklist_mutex );
    m_blacklisted_games.erase( name );
    save( );
}

// savemgr-ignore
std::optional<IgnoreRule> Blacklist::parse_ignore_line( std::string_view l ) {
    auto trimmed = utils::trim( l );

    if ( trimmed.empty( ) || trimmed.starts_with( "#" ) ) return std::nullopt;

    if ( trimmed.ends_with( "/" ) ) {
        auto pos = trimmed.rfind( "/" );
        return IgnoreRule{ IgnoreKind::Directory, std::string( trimmed.substr( 0, pos ) ) };
    } else if ( trimmed.starts_with( "*." ) ) {
        return IgnoreRule{ IgnoreKind::Extension, std::string( trimmed.substr( 1 ) ) };
    }

    return IgnoreRule{ IgnoreKind::Filename, std::string( trimmed ) };
}

std::vector<IgnoreRule> Blacklist::parse_ignore_file( const fs::path& f ) {
    std::vector<IgnoreRule> rules = { };

    std::ifstream file( f );
    if ( !file.is_open( ) ) return rules;

    std::string line;
    while ( std::getline( file, line ) ) {
        auto rule = parse_ignore_line( line );
        if ( rule.has_value( ) ) {
            rules.emplace_back( rule.value( ) );
        }
    }

    return rules;
}

bool Blacklist::is_ignored( const fs::path& file, const std::vector<IgnoreRule>& rules ) {
    for ( const auto& rule : rules ) {
        switch ( rule.kind ) {
        case IgnoreKind::Extension:
            if ( file.extension( ).string( ) == rule.value ) return true;
            break;
        case IgnoreKind::Filename:
            if ( file.filename( ).string( ) == rule.value ) return true;
            break;
        case IgnoreKind::Directory:
            for ( const auto& part : file ) {
                if ( part.string( ) == rule.value ) return true;
            }
            break;
        }
    }
    return false;
}
