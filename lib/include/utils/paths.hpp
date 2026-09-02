#pragma once
#include <branding.hpp>

#if defined( __linux__ )
#include <unistd.h>
#include <climits>
#elif defined( __APPLE__ )
#include <mach-o/dyld.h>
#include <climits>
#endif

namespace fs = std::filesystem;

namespace paths {
    inline fs::path g_config_dir;
    inline void set_config_dir( const fs::path& p ) { g_config_dir = p; }

    inline fs::path exe_dir() {
#if defined( _WIN32 )
        wchar_t szFileName[MAX_PATH];
        GetModuleFileNameW( NULL, szFileName, MAX_PATH );
        fs::path exe_path( szFileName );
        return exe_path.parent_path( );
#elif defined( __linux__ )
        char buffer[PATH_MAX];
        ssize_t len = readlink( "/proc/self/exe", buffer, sizeof( buffer ) - 1 );
        if ( len == -1 ) throw std::runtime_error( "readlink /proc/self/exe failed" );
        buffer[len] = '\0';
        return fs::path( buffer );
#elif defined( __APPLE__ )
        char buffer[PATH_MAX];
        uint32_t size = sizeof( buffer );
        if ( _NSGetExecutablePath( buffer, &size ) != 0 ) throw std::runtime_error( "exe path buffer too small" );
        return fs::canonical( fs::path( buffer ) );
#endif
    }

    inline fs::path home_dir( ) {
        const char* home;
#if defined( __linux__ ) || defined( __APPLE__ )
        home = std::getenv( "HOME" );
#elif defined( _WIN32 )
        home = std::getenv( "USERPROFILE" );
#endif
        if ( !home ) throw std::runtime_error( "HOME not set, how did you manage to do this?" );
        return fs::path( home );
    }

    inline fs::path default_config_dir( ) {


#if defined( __linux__ )
        return home_dir( ) / ".config" / APP_NAME;
#elif defined( __APPLE__ )
        return home_dir( ) / "Library" / "Application Support" / APP_NAME;
#elif defined( _WIN32 )
        return home_dir( ) / "AppData" / "Roaming" / APP_NAME;
#endif
    }

    inline fs::path config_dir( ) {
        if ( !g_config_dir.empty( ) ) {
            return g_config_dir;
        }

        return default_config_dir( );
    }

    // redirects the config location if present - TODO: verify this
    inline fs::path redirect_file( ) { return config_dir( ) / "cfg.redirect"; }

    inline fs::path log_dir( ) { return config_dir( ) / "logs"; }
    inline fs::path log_file( ) { return log_dir( ) / std::format( "{}.log", APP_NAME ); }

    inline fs::path cache_dir( ) { return config_dir( ) / "cache"; }
    inline fs::path backgrounds_dir( ) { return config_dir( ) / "backgrounds"; }

    inline fs::path demo_disk_cache_file( ) { return config_dir( ) / "demo_cache.json"; }

    inline fs::path documents_dir( ) {
#if defined( _WIN32 )
        PWSTR path = NULL;
        HRESULT h_res = SHGetKnownFolderPath( FOLDERID_Documents, 0, NULL, &path );
        if ( SUCCEEDED( h_res ) ) {
            fs::path result( path );
            CoTaskMemFree( path );
            return result;
        } else {
            throw std::runtime_error( "USERPROFILE not set, how did you manage to do this?" );
        }
#endif
        return home_dir( ) / "Documents";
    }
}; // namespace paths
