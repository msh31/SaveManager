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

/*
    TODO LIST
    
    1. figure out if the direct file  works
    2. decide to keep said redirect file
    3. currently overriden in GUI's entrypoint - should be guarded by an ifdef like 'SM_PORTABLE' or some shit 
    4. not always the default directory, to be verified
*/

namespace paths {
    inline fs::path g_config_dir = { };
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

    //3. 
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
 
    inline fs::path redirect_file( ) { return config_dir( ) / "cfg.redirect"; } // 1. & 2.

    inline fs::path log_dir( ) { return config_dir( ) / "logs"; }
    inline fs::path log_file( ) { return log_dir( ) / std::format( "{}.log", APP_NAME ); }

    inline fs::path cache_dir( ) { return config_dir( ) / "cache"; }
    inline fs::path backgrounds_dir( ) { return config_dir( ) / "backgrounds"; }
    inline fs::path backup_dir( ) { return config_dir( ) / "backups"; }
    inline fs::path plugin_dir( ) { return config_dir( ) / "plugins"; }
    inline fs::path ubi_translations( ) { return config_dir( ) / "ubi_translations.json"; }
    inline fs::path pcgw_manifest( ) { return config_dir( ) / "pcgw_manifest.json"; }
    inline fs::path blacklist( ) { return config_dir( ) / "game_blacklist.json"; }

    inline fs::path demo_disk_cache_file( ) { return cache_dir( ) / "demo_cache.json"; };
    inline fs::path unreal_name_cache( ) { return cache_dir( ) / "unreal_names.json"; }

// le games
#if defined( __linux__ )
    inline fs::path lutris_dir( ) { return home_dir( ) / "Games"; } //4. 
#endif
    inline fs::path heroic_dir( ) { return home_dir( ) / "Games" / "Heroic"; } //4. 

//ugly platform guarded herlpers
#if defined( _WIN32 )
    inline fs::path get_known_folder_path( const KNOWNFOLDERID& folder_id ) {
        PWSTR path = NULL;
        HRESULT h_res = SHGetKnownFolderPath( folder_id, 0, NULL, &path );
        if ( SUCCEEDED( h_res ) ) {
            fs::path result( path );
            CoTaskMemFree( path );
            return result;
        } else {
            // what is logged here is the GUID of a known folder like FOLDERID_Documents or whatever,
            //  this is a stupid windows type so we're printing this shit raw
            auto str = std::format(
                "Failed to find known folder {:08X}-{:04X}-{:04X}-{:02X}{:02X}-{:02X}{:02X}{:02X}{:02X}{:02X}{:02X}, "
                "how did you manage this?",
                folder_id.Data1, folder_id.Data2, folder_id.Data3, folder_id.Data4[0], folder_id.Data4[1],
                folder_id.Data4[2], folder_id.Data4[3], folder_id.Data4[4], folder_id.Data4[5], folder_id.Data4[6],
                folder_id.Data4[7] );
            throw std::runtime_error( str );
        }
    }
#endif

    inline fs::path documents_dir( ) {
#if defined( _WIN32 )
        return get_known_folder_path( FOLDERID_Documents );
#endif
        return home_dir( ) / "Documents";
    }

#if defined( __linux__ )
    //fallbacks match the spec when env is not set
    inline fs::path xdg_data_home( ) {
        const char* path = std::getenv( "XDG_DATA_HOME" );
        if ( path ) {
            return fs::path( path );
        } else {
            return home_dir( ) / ".local" / "share";
        }
        return { };
    }

    inline fs::path xdg_config_home( ) {
        const char* path = std::getenv( "XDG_CONFIG_HOME" );
        if ( path ) {
            return fs::path( path );
        } else {
            return home_dir( ) / ".config";
        }
    }
#endif

#if defined( _WIN32 )
    inline fs::path win_dir( ) {
        const char* path = std::getenv( "WINDIR" );
        if ( path ) {
            return fs::path( path );
        } else {
            return fs::path( "C:/Windows" ); // fallback, WINDIR should basically always be set
        }
    }
#endif

}; // namespace paths
