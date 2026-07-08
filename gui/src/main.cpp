#include "app/app.hpp"
#include <backend/window/window_manager.hpp>

#include <utils/blacklist/blacklist.hpp>
// #include <utils/steam/steam.hpp>
#include <utils/translations/translations.hpp>

#ifdef _WIN32 // forces Windows to treat the app as a GUI Application
    #pragma comment( linker, "/subsystem:windows /entry:mainCRTStartup" )
#endif

int main( ) {
    try {
        init_logger( "[%n]: [%l] %d-%m-%Y %H:%M:%S - %v" );

        Translations translations;
        Blacklist blacklist;
        CWindowManager window;
        if ( !translations.init( ) ) {
            SPDLOG_WARN( "Failed to initialize translations! Expect missing games!" );
        }
        if ( !blacklist.init( ) ) {
            SPDLOG_WARN( "Failed to initialize blacklist!" );
        }

        CApp app( blacklist, translations );
        app.init( );
        SPDLOG_INFO( "Initialized succesfully!" );
        window.run( [&] { app.render_shader( window.get_size( ) ); }, [&app] { app.render( ); } );
    } catch ( const std::exception& e ) {
        SPDLOG_CRITICAL( "Fatal: {}", e.what( ) );
        return 1;
    }
    return 0;
}
