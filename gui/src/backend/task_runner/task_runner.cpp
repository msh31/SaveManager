#include "task_runner.hpp"

void CTaskRunner::run(
    std::function<void( )> work, std::function<void( )> on_complete,
    std::function<void( const std::exception& )> on_error ) {
    m_tasks.emplace_back(
        Task{ std::async( std::launch::async, work ), std::move( on_complete ), std::move( on_error ) } );
}

void CTaskRunner::update( ) {
    std::erase_if( m_tasks, []( Task& t ) {
        bool ready = false;
        if ( t.future.valid( ) && t.future.wait_for( std::chrono::seconds( 0 ) ) == std::future_status::ready ) {
            ready = true;
        }
        if ( ready ) {
            try {
                t.future.get( );
                t.on_complete( );
                return true;
            } catch ( const std::exception& ex ) {
                t.on_error( ex );
                return true;
            }
        }
        return false;
    } );
}

// COMMENT: work runs on a background thread and may outlive the owning object on shutdown
// NEVER capture `this` or references to view/owner state, only owned/copied data..
void CTaskRunner::shutdown( ) {
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
