#include "utils/steam/steam.hpp"
#include "utils/paths.hpp"

std::vector<std::string> SteamHelper::get_platform_steam_paths( ) {
#ifdef __APPLE__
    return {
        paths::home_dir( ) / "Library" / "Application Support" / "Steam" / "steamapps" / "libraryfolders.vdf",
    };
#endif

    try {
#ifdef __linux__
        return {
            paths::home_dir( ).string( ) + "/.steam/steam/steamapps/libraryfolders.vdf",
            paths::home_dir( ).string( ) + "/.local/share/Steam/steamapps/libraryfolders.vdf" };
#endif

#ifdef _WIN32
        return {
            "C:\\Program Files (x86)\\Steam\\steamapps\\libraryfolders.vdf",
        };
#endif
    } catch ( const std::exception& e ) {
        SPDLOG_ERROR( "Failed to get Steam paths: {}", std::string( e.what( ) ) );
        return { };
    }
}

std::optional<fs::path> SteamHelper::get_steam_location( ) {
    for ( const auto& entry : get_platform_steam_paths( ) ) {
        if ( fs::exists( entry ) ) {
            return entry;
        }
    }
    SPDLOG_ERROR( "Failed to find steam location!" );
    return std::nullopt;
}

std::optional<std::string> SteamHelper::parse_steam_userid( ) {
    auto base = get_steam_location( );
    if ( base == std::nullopt ) return std::nullopt;

    auto steam_root = base->parent_path( ).parent_path( );
    auto loginusers_vdf = steam_root / "config" / "loginusers.vdf";

    std::ifstream file( loginusers_vdf.string( ) );
    std::string line;

    if ( !file.is_open( ) ) {
        SPDLOG_ERROR( "Failed to open loginusers.vdf for parsing!" );
        return { };
    }

    std::string current_id = { };
    while ( std::getline( file, line ) ) {
        if ( line.find( "\"7656119" ) != std::string::npos ) {
            auto last_close = line.rfind( '"' );
            auto last_open = line.rfind( '"', last_close - 1 );

            std::string value = line.substr( last_open + 1, last_close - last_open - 1 );

            current_id = value;
            continue;
        }
        if ( line.find( "\"MostRecent\"" ) != std::string::npos ) {
            auto last_close = line.rfind( '"' );
            auto last_open = line.rfind( '"', last_close - 1 );

            std::string value = line.substr( last_open + 1, last_close - last_open - 1 );
            if ( value == "1" ) return current_id;
            else
                continue;
        }
    }

    file.close( );
    return std::nullopt;
}

std::vector<fs::path> SteamHelper::get_library_folders( ) {
    auto vdf_file = get_steam_location( );
    std::vector<fs::path> libraries;

    if ( !vdf_file ) {
        SPDLOG_WARN( "Steam installation not found" );
        return { };
    }

    std::ifstream file( vdf_file.value( ).string( ) );
    std::string line;

    if ( !file.is_open( ) ) {
        SPDLOG_ERROR( "Failed to open Steam library file" );
        return { };
    }

    while ( std::getline( file, line ) ) {
        if ( line.find( "\"path\"" ) != std::string::npos ) {
            size_t first_quote = line.find( '"' );
            size_t second_quote = line.find( '"', first_quote + 1 );
            size_t third_quote = line.find( '"', second_quote + 1 );
            size_t fourth_quote = line.find( '"', third_quote + 1 );

            if ( fourth_quote == std::string::npos ) {
                continue;
            }

            std::string path_value = line.substr( third_quote + 1, fourth_quote - third_quote - 1 );
            libraries.push_back( path_value );
        }
    }

    file.close( );
    return libraries;
}

std::optional<SteamManifest> SteamHelper::parse_app_manifest( const fs::path& acf_path ) {
    uint32_t appid = 0;
    std::string name;
    std::string install_dir;

    std::ifstream f( acf_path.string( ) );
    if ( !f.is_open( ) ) {
        SPDLOG_ERROR( "Failed to parse app manifest" );
        return { };
    }

    std::string str;
    while ( std::getline( f, str ) ) {
        if ( str.find( "\"appid\"" ) != std::string::npos ) {
            auto last_close = str.rfind( '"' );
            auto last_open = str.rfind( '"', last_close - 1 );

            std::string value = str.substr( last_open + 1, last_close - last_open - 1 );

            appid = std::stoul( value );
        }
        if ( str.find( "\"name\"" ) != std::string::npos ) {
            auto last_close = str.rfind( '"' );
            auto last_open = str.rfind( '"', last_close - 1 );

            std::string value = str.substr( last_open + 1, last_close - last_open - 1 );

            name = value;
        }
        if ( str.find( "\"installdir\"" ) != std::string::npos ) {
            auto last_close = str.rfind( '"' );
            auto last_open = str.rfind( '"', last_close - 1 );

            std::string value = str.substr( last_open + 1, last_close - last_open - 1 );

            install_dir = value;
        }
    }

    if ( appid == 0 || name.empty( ) || install_dir.empty( ) ) return std::nullopt;
    return SteamManifest{ appid, name, install_dir };
}

// // SteamManifestCache
// bool SteamManifestCache::init( ) {
//     auto libraries = SteamHelper::get_library_folders( );
//     if ( libraries.empty( ) ) return false;
//
//     for ( const auto& library : libraries ) {
//         for ( const auto& entry :
//               fs::directory_iterator( library / "steamapps", fs::directory_options::skip_permission_denied ) ) {
//             if ( entry.is_directory( ) ) continue;
//             if ( entry.path( ).extension( ) != ".acf" ) continue;
//
//             if ( auto manifest = SteamHelper::parse_app_manifest( entry.path( ) ) ) {
//                 manifest->library_dir = library;
//                 m_cache.emplace( manifest->appid, *manifest );
//             }
//         }
//     }
//     if ( m_cache.empty( ) ) return false;
//     return true;
// }
//
// const std::unordered_map<uint32_t, SteamManifest>& SteamManifestCache::get_app_manifests( ) { return m_cache; }
