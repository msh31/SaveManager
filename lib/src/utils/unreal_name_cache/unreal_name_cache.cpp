#include "utils/unreal_name_cache/unreal_name_cache.hpp"
#include "utils/paths.hpp"

bool UnrealNameCache::init( ) {
    std::lock_guard<std::mutex> lock( m_mutex );

    auto file_path = paths::unreal_name_cache( );
    if ( !fs::exists( file_path ) ) return true;

    std::ifstream file( file_path.c_str( ) );
    if ( !file.is_open( ) ) {
        SPDLOG_WARN( "Failed to open unreal name cache for reading!" );
        return false;
    }
    if ( fs::file_size( file_path ) == 0 ) return true;

    try {
        json data = json::parse( file );

        if ( data.contains( "appid" ) || data.contains( "folder" ) ) {
            for ( const auto& [appid_str, name] : data.value( "appid", json::object( ) ).items( ) )
                m_cache[static_cast<uint32_t>( std::stoul( appid_str ) )] = name.get<std::string>( );
            for ( const auto& [folder, name] : data.value( "folder", json::object( ) ).items( ) )
                m_folder_cache[folder] = name.get<std::string>( );
        } else {
            // legacy flat appid -> name format
            for ( const auto& [appid_str, name] : data.items( ) )
                m_cache[static_cast<uint32_t>( std::stoul( appid_str ) )] = name.get<std::string>( );
        }
    } catch ( const std::exception& ex ) {
        SPDLOG_WARN( "Failed to parse unreal name cache: {}", ex.what( ) );
        file.close( );
        return false;
    }
    file.close( );
    return true;
}

std::optional<std::string> UnrealNameCache::get( uint32_t appid ) const {
    std::lock_guard<std::mutex> lock( m_mutex );
    if ( auto it = m_cache.find( appid ); it != m_cache.end( ) ) return it->second;
    return std::nullopt;
}

void UnrealNameCache::remember( uint32_t appid, const std::string& name ) {
    {
        std::lock_guard<std::mutex> lock( m_mutex );
        auto it = m_cache.find( appid );
        if ( it != m_cache.end( ) && it->second == name ) return;
        m_cache[appid] = name;
    }
    save( );
}

std::optional<std::string> UnrealNameCache::get_by_folder( const std::string& folder_name ) const {
    std::lock_guard<std::mutex> lock( m_mutex );
    if ( auto it = m_folder_cache.find( folder_name ); it != m_folder_cache.end( ) ) return it->second;
    return std::nullopt;
}

void UnrealNameCache::remember_by_folder( const std::string& folder_name, const std::string& name ) {
    {
        std::lock_guard<std::mutex> lock( m_mutex );
        auto it = m_folder_cache.find( folder_name );
        if ( it != m_folder_cache.end( ) && it->second == name ) return;
        m_folder_cache[folder_name] = name;
    }
    save( );
}

void UnrealNameCache::save( ) const {
    std::lock_guard<std::mutex> lock( m_mutex );

    json data;
    for ( const auto& [appid, name] : m_cache )
        data["appid"][std::to_string( appid )] = name;
    for ( const auto& [folder, name] : m_folder_cache )
        data["folder"][folder] = name;

    std::ofstream file( paths::unreal_name_cache( ) );
    if ( !file.is_open( ) ) {
        SPDLOG_WARN( "Failed to open unreal name cache for writing!" );
        return;
    }
    file << data.dump( 4 );
    file.close( );
}
