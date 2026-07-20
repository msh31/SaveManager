#include "detection_service.hpp"

void CDetectionService::refresh( ) {
    if ( is_refreshing( ) ) return;

    auto start = std::chrono::steady_clock::now( );
    m_future = std::async( std::launch::async, [this, start] {
        auto result = Detection::find_saves( m_blacklist, m_translations, m_manifest_cache, m_name_cache );
        {
            std::lock_guard lock( m_mutex );
            m_result = std::move( result );
        }
        m_last_duration.store( std::chrono::duration<double>( std::chrono::steady_clock::now( ) - start ).count( ) );
        m_generation.fetch_add( 1 );
    } );
}

void CDetectionService::ensure_started( ) {
    if ( m_generation.load( ) == 0 && !is_refreshing( ) ) refresh( );
}

bool CDetectionService::is_refreshing( ) const {
    return m_future.valid( ) && m_future.wait_for( std::chrono::seconds( 0 ) ) != std::future_status::ready;
}

std::vector<Game> CDetectionService::snapshot( ) const {
    std::lock_guard lock( m_mutex );
    return m_result;
}
