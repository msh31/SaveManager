#include "task_runner.hpp"

void CTaskRunner::run( std::function<void( )> work, std::function<void( )> on_complete ) {
    m_tasks.emplace_back( Task{ std::async( std::launch::async, work ), std::move( on_complete ) } );
}

void CTaskRunner::update( ) {
    std::erase_if( m_tasks, []( Task& t ) {
        if ( t.future.valid( ) && t.future.wait_for( std::chrono::seconds( 0 ) ) == std::future_status::ready ) {
            t.on_complete( );
            return true;
        }
        return false;
    } );
}

void CTaskRunner::shutdown( ) {
    for ( auto& task : m_tasks ) {
        bool is_valid_and_ready =
            task.future.valid( ) && task.future.wait_for( std::chrono::seconds( 0 ) ) != std::future_status::ready;

        if ( is_valid_and_ready ) {
            std::thread thread( [f = std::move( task.future )] { f.wait( ); } );
            thread.detach( );
        }
    }
    m_tasks.clear( );
}
