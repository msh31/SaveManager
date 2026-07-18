#pragma once
namespace fs = std::filesystem;

#if defined( _WIN32 )
    #include <KnownFolders.h>
    #include <shlobj.h>
    #include <wchar.h>
    #include <windows.h>
#endif

namespace paths {
    inline fs::path g_config_dir;
    inline void set_config_dir( const fs::path& p ) { g_config_dir = p; }

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
        std::string folder_name = "savemanager";

#ifndef NDEBUG
        folder_name = folder_name + "-dev";
#endif // NDEBUG

#if defined( __linux__ )
        return home_dir( ) / ".config" / folder_name;
#elif defined( __APPLE__ )
        return home_dir( ) / "Library" / "Application Support" / folder_name;
#elif defined( _WIN32 )
        return home_dir( ) / folder_name;
#endif
    }

    inline fs::path config_dir( ) {
        if ( !g_config_dir.empty( ) ) {
            return g_config_dir;
        }

        return default_config_dir( );
    }

    inline fs::path schedule_file( ) { return default_config_dir( ) / "schedule.json"; }

    inline fs::path redirect_file( ) { return default_config_dir( ) / "cfg.redirect"; }

    inline fs::path backup_dir( ) { return config_dir( ) / "backups"; }

    inline fs::path plugin_dir( ) { return config_dir( ) / "plugins"; }

    inline fs::path cache_dir( ) { return config_dir( ) / "cache"; }

    inline fs::path log_dir( ) { return config_dir( ) / "logs"; }

#if defined( __linux__ )
    inline fs::path lutris_dir( ) { return home_dir( ) / "Games"; }
#endif

    inline fs::path heroic_dir( ) { return home_dir( ) / "Games" / "Heroic"; }

#if defined( _WIN32 )
    inline fs::path get_known_folder_path( const KNOWNFOLDERID& folder_id ) {
        PWSTR path = NULL;
        HRESULT h_res = SHGetKnownFolderPath( folder_id, 0, NULL, &path );
        if ( SUCCEEDED( h_res ) ) {
            fs::path result( path );
            CoTaskMemFree( path );
            return result;
        } else {
            //what is logged here is the GUID of a known folder like FOLDERID_Documents or whatever, 
            // this is a stupid windows type so we're printing this shit raw
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

    //TODO: dont call this in detectors anymore - its the literal host documetns folder
    inline fs::path documents_dir( ) {
#if defined( _WIN32 )
        return get_known_folder_path( FOLDERID_Documents );
#endif
        return home_dir( ) / "Documents";
    }

    inline fs::path ubi_translations( ) { return config_dir( ) / "ubi_translations.json"; }
    inline fs::path steam_appids( ) { return config_dir( ) / "steamids.json"; }
    inline fs::path blacklist( ) { return config_dir( ) / "game_blacklist.json"; }

    inline fs::path socket( ) { return fs::temp_directory_path( ) / "savemanager.sock"; }
}; // namespace paths
