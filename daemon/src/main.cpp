#include "watcher/watcher.hpp"
#include <server/server.hpp>

#if defined( __linux__ )
    #include <sys/inotify.h>
#endif

#include <config/config.hpp>
#include <constants.hpp>
#include <logger/logger.hpp>
#include <utils/paths.hpp>

int main( ) {
    CConfig config;
    CServer m_server;

    spdlog::set_level( spdlog::level::debug );
    init_logger( "[%n]: [%l] %d-%m-%Y %H:%M:%S - %v", "savemanager-daemon" );
    SPDLOG_INFO( "daemon started!" );

#if defined( __linux__ )
    CWatcher watcher(
        []( const fs::path& path, uint32_t mask ) {
            if ( !fs::exists( path ) ) SPDLOG_WARN( "{} does not exist", path.string( ) );
            auto ext = path.extension( ).string( );

            if ( extension_blocklist.contains( ext ) ) return;
            if ( g_extension_blocklist.contains( ext ) ) return;

            if ( mask & IN_CREATE ) SPDLOG_INFO( "created: {}", path.string( ) );
            else if ( mask & IN_DELETE )
                SPDLOG_INFO( "deleted: {}", path.string( ) );
            else if ( mask & IN_CLOSE_WRITE )
                SPDLOG_INFO( "modified: {}", path.string( ) );
        },
        config );

    for ( auto& entry : config.settings.watch_paths ) {
        if ( !watcher.add_watch( entry ) ) {
            SPDLOG_ERROR( "failed to add watcher for: {}", entry.string( ) );
        }
    }
    std::thread watcher_thread( &CWatcher::run, &watcher );
    watcher_thread.detach( );
#endif

    std::thread server_thread( &CServer::run, &m_server );
    server_thread.detach( );

    while ( true ) {
        std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    }

    return 0;
}
