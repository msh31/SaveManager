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

        const auto& saved_props = app.window_props( );
        window.restore_state( saved_props.x, saved_props.y, saved_props.width, saved_props.height );
        window.show( );

        app.init( );
        SPDLOG_INFO( "Initialized succesfully!" );
        window.run( [&] { app.render_shader( window.get_size( ) ); }, [&app] { app.render( ); } );

        const auto [pos_x, pos_y] = window.get_pos( );
        const auto [width, height] = window.get_window_size( );
        app.save_window_props( pos_x, pos_y, width, height );
    } catch ( const std::exception& e ) {
        SPDLOG_CRITICAL( "Fatal: {}", e.what( ) );
        return 1;
    }
    return 0;
}
