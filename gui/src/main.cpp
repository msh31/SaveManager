#include "app/app.hpp"
#include <backend/window/window_manager.hpp>

#ifdef _WIN32 // forces Windows to treat the app as a GUI Application
    #pragma comment( linker, "/subsystem:windows /entry:mainCRTStartup" )
#endif

int main( ) {
    try {
        init_logger( "[%n]: [%l] %d-%m-%Y %H:%M:%S - %v" );

        CWindowManager window;
        CApp app;

        app.init( );
        SPDLOG_INFO( "Initialized succesfully!" );

        window.run( [&] { app.render_shader( window.get_size( ) ); }, [&app] { app.render( ); } );
    } catch ( const std::exception& e ) {
        SPDLOG_CRITICAL( "Fatal: {}", e.what( ) );
        return 1;
    }
    return 0;
}
