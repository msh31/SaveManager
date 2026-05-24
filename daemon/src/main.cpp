#include "watcher.hpp"

#if defined( __linux__ )
#include <sys/inotify.h>
#endif

#include <config/config.hpp>
#include <constants.hpp>
#include <logger/logger.hpp>
#include <utils/paths.hpp>

#include <features/scheduler/scheduler.hpp>

int main( ) {
    CConfig config;

    SaveScheduler scheduler( config );

    spdlog::set_level( spdlog::level::debug );
    init_logger( "[%n]: [%l] %d-%m-%Y %H:%M:%S - %v", "savemanager-daemon" );
    SPDLOG_INFO( "daemon started!" );

#if defined( __linux__ )
    Watcher watcher(
        []( const fs::path &path, uint32_t mask ) {
            if ( !fs::exists( path ) ) SPDLOG_WARN( "{} does not exist", path.string( ) );
            auto ext = path.extension( ).string( );
            if ( std::ranges::contains( extension_blocklist, ext ) ||
                 std::ranges::contains( g_extension_blocklist, ext ) )
                return;

            if ( mask & IN_CREATE ) SPDLOG_INFO( "created: {}", path.string( ) );
            else if ( mask & IN_DELETE )
                SPDLOG_INFO( "deleted: {}", path.string( ) );
            else if ( mask & IN_CLOSE_WRITE )
                SPDLOG_INFO( "modified: {}", path.string( ) );
        },
        config );

    for ( auto &entry : config.settings.watch_paths ) {
        if ( !watcher.add_watch( entry ) ) {
            SPDLOG_ERROR( "failed to add watcher for: {}", entry.string( ) );
        }
    }
    scheduler.run( );
    watcher.run( );
#else
    return 0;
#endif
}
