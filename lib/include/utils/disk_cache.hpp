#pragma once
#include "cache.hpp"
#include <utils/utils.hpp>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

template <typename T> class CDiskCache {
    public:
        CCache<T> m_cache;
        fs::path m_file;

        CDiskCache( fs::path cache ) {
            m_file = cache;
            m_cache.set_on_updated( [this]( const T& val ) { save( val ); } );
            if ( !load( ) ) {
                SPDLOG_ERROR( "[DiskCache]: Failed to load cache from: {}", cache.string( ) );
                return;
            }
        }

        bool load( ) {
            if ( !fs::exists( m_file ) ) {
                SPDLOG_ERROR( "[DiskCache]: The provided path does not exist!" );
                return false;
            }

            std::ifstream file( m_file.c_str( ) );
            if ( !file.is_open( ) ) {
                SPDLOG_ERROR( "[DiskCache]: Failed to open the provided cache file" );
                return false;
            }

            json data;
            try {
                data = json::parse( file );
                m_cache.seed( data.get<T>( ) );
                return true;
            } catch ( json::exception& ex ) {
                SPDLOG_CRITICAL( std::format( "[DiskCache] parsing error: {}", ex.what( ) ) );
            }
            return false;
        }

        bool save( const T& val ) {
            json data = val;
            return utils::atomic_write( m_file, data.dump( 4 ) );
        }

        void refresh( std::function<T( TaskControl& )> fun ) { m_cache.refresh( fun ); }

        const T& get( ) { return m_cache.get( ); }
        bool is_refreshing( ) { return m_cache.is_refreshing( ); }

        float progress( ) { return m_cache.progress( ); }
        void request_cancel( ) { m_cache.request_cancel( ); }

        CDiskCache( const CDiskCache& ) = delete;
        CDiskCache& operator=( const CDiskCache& ) = delete;
        CDiskCache( CDiskCache&& ) = delete;
        CDiskCache& operator=( CDiskCache&& ) = delete;

    private:
        // T m_current_snapshot = { };
};