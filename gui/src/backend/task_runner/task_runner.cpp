#include "task_runner.hpp"

void CTaskRunner::run( std::function<void( )> work, std::function<void( )> on_complete ) {
    m_tasks.emplace_back( Task{ std::async( std::launch::async, work ), on_complete } );
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
