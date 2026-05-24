#pragma once

inline std::string get_username( ) {
#if defined( __linux__ ) || defined( __APPLE__ )
    return std::getenv( "USER" );
#else
    return std::getenv( "USERNAME" );
#endif
}
