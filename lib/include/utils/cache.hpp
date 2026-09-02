#pragma once
#include <logger.hpp>
#include <async_queue/async_queue.hpp>

template <typename T> class CCache {
    public:
        ~CCache( ) { m_queue.shutdown( ); }

        void refresh( std::function<T( TaskControl& )> fun ) {
            bool expected = false;
            if ( !m_is_refreshing.compare_exchange_strong( expected, true ) ) return;

            m_task_handle = m_queue.run<T>(
                fun,
                [this]( T val ) {
                    m_current_snapshot = val;
                    m_is_refreshing = false;
                    if ( m_on_updated ) m_on_updated( m_current_snapshot );
                },
                [this]( const std::exception& ex ) {
                    SPDLOG_ERROR( "Cache Refresh Error: {}", ex.what( ) );
                    m_is_refreshing = false;
                } );
        }
        const T& get( ) {
            m_queue.update( );
            return m_current_snapshot;
        }
        bool is_refreshing( ) { return m_is_refreshing; }
        void seed( T val ) { m_current_snapshot = val; }
        void set_on_updated( std::function<void( const T& )> fn ) { m_on_updated = fn; }

        float progress( ) {
            if ( !m_task_handle.has_value( ) ) return 0.0f;
            else
                return m_task_handle->progress( );
        }
        void request_cancel( ) {
            if ( m_task_handle ) m_task_handle->request_cancel( );
        }

    private:
        T m_current_snapshot = { };
        std::atomic<bool> m_is_refreshing = false;
        std::function<void( const T& )> m_on_updated;

        CAsyncQueue m_queue;
        std::optional<TaskHandle> m_task_handle;
};
