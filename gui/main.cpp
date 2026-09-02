#include <backend/app.hpp>
#include <backend/window_manager/window_manager.hpp>
#include <curl/curl.h>
#include <logger.hpp>

#ifdef _WIN32 // forces Windows to treat the app as a GUI Application
    #pragma comment( linker, "/subsystem:windows /entry:mainCRTStartup" )
#endif

int main( ) {
    curl_global_init( CURL_GLOBAL_ALL );
    try {
        setup_logger( );
        paths::set_config_dir( paths::exe_dir( ) / "config" );

        SPDLOG_INFO( "==============={}===============", APP_NAME );
        CConfig config;
        CWindowManager window( config );
        CApp app( config );

        app.init( );
        window.set_drop_callback( [&app]( const auto& paths ) { app.on_files_dropped( paths ); } );
        SPDLOG_INFO( "Initialized succesfully!" );

        window.run( [&app] { app.render( ); } );
    } catch ( const std::exception& e ) {
        SPDLOG_CRITICAL( "Fatal: {}", e.what( ) );
        curl_global_cleanup( );
        return 1;
    }

    curl_global_cleanup( );
    return 0;
}
