#include <async_queue/async_queue.hpp>

void CAsyncQueue::update( ) {
    std::vector<Task> ready_tasks;

    auto it = std::remove_if( m_tasks.begin( ), m_tasks.end( ), [&ready_tasks]( Task& t ) {
        bool ready = t.future.valid( ) && t.future.wait_for( std::chrono::seconds( 0 ) ) == std::future_status::ready;
        if ( ready ) {
            ready_tasks.push_back( std::move( t ) );
        }
        return ready;
    } );
    m_tasks.erase( it, m_tasks.end( ) );

    for ( auto& t : ready_tasks ) {
        try {
            t.future.get( );
            t.on_complete( );
        } catch ( const std::exception& ex ) {
            t.on_error( ex );
        }
    }
}

// COMMENT: work runs on a background thread and may outlive the owning object on shutdown
// NEVER capture `this` or references to view/owner state, only owned/copied data..
void CAsyncQueue::shutdown( ) {
    for ( auto& task : m_tasks ) {
        bool is_future_busy =
            task.future.valid( ) && task.future.wait_for( std::chrono::seconds( 0 ) ) != std::future_status::ready;

        if ( is_future_busy ) {
            std::thread thread( [f = std::move( task.future )] { f.wait( ); } );
            thread.detach( );
        }
    }
    m_tasks.clear( );
}