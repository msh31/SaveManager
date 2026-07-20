#include "save_helper.hpp"
#include <utils/steam/steam.hpp>

fs::path save::resolve_root( SaveRoot sr ) {
#if defined( _WIN32 )
    switch ( sr ) {
    case SaveRoot::DOCUMENTS:
        return paths::get_known_folder_path( FOLDERID_Documents );
        break;
    case SaveRoot::LOCAL_APPDATA:
        return paths::get_known_folder_path( FOLDERID_LocalAppData );
    case SaveRoot::LOCAL_APPDATA_LOW:
        return paths::get_known_folder_path( FOLDERID_LocalAppDataLow );
        break;
    case SaveRoot::PROGRAM_DATA:
        return paths::get_known_folder_path( FOLDERID_ProgramData );
        break;
    case SaveRoot::SAVED_GAMES:
        return paths::get_known_folder_path( FOLDERID_SavedGames );
        break;
    case SaveRoot::APPDATA:
        return paths::get_known_folder_path( FOLDERID_RoamingAppData );
        break;
    case SaveRoot::USER_PROFILE:
        return paths::get_known_folder_path( FOLDERID_Profile );
        break;
    case SaveRoot::PROGRAM_FILES:
        return paths::get_known_folder_path( FOLDERID_ProgramFiles );
        break;
    case SaveRoot::STEAM_DIR:
        return SteamHelper::get_steam_location( ).value_or( fs::path{ } );
        break;
    default:
        return { };
    }
#endif
#if defined( __linux__ )
    switch ( sr ) {
    case SaveRoot::XDG_DATA_HOME:
        return paths::xdg_data_home( );
        break;
    case SaveRoot::XDG_CONFIG_HOME:
        return paths::xdg_config_home( );
        break;
    case SaveRoot::LINUX_HOME:
        return paths::home_dir( );
        break;
    case SaveRoot::STEAM_DIR:
        return SteamHelper::get_steam_location( ).value_or( fs::path{ } );
        break;
    default:
        return { };
    }
#endif
#if defined( __APPLE__ )
    switch ( sr ) {
    case SaveRoot::OSX_HOME:
        return paths::home_dir( );
        break;
    case SaveRoot::STEAM_DIR:
        return SteamHelper::get_steam_location( ).value_or( fs::path{ } );
        break;
    default:
        return { };
    }
#endif

    return { };
}

namespace {
    bool folder_contains_header( const fs::path& path, const std::array<char, 4>& header ) {
        try {
            for ( const auto& entry :
                  fs::recursive_directory_iterator( path, fs::directory_options::skip_permission_denied ) ) {
                if ( !fs::is_regular_file( entry ) ) continue;

                std::ifstream file( entry.path( ), std::ifstream::binary );
                if ( !file.is_open( ) ) continue;

                std::array<char, 4> buffer{ };
                file.read( buffer.data( ), 4 );
                if ( file.gcount( ) == 4 && buffer == header ) return true;
            }
        } catch ( const fs::filesystem_error& ex ) {
            SPDLOG_WARN( "failed to scan {}: {}", path.string( ), ex.what( ) );
        }
        return false;
    }
} // namespace

std::vector<Game> save::scan_locations(
    const std::unordered_map<SaveRoot, fs::path>& roots, const std::vector<SaveLocation>& table, PlatformType type,
    std::string_view platform_label ) {
    std::vector<Game> games;

    for ( const auto& loc : table ) {
        auto root_it = roots.find( loc.root_path );
        if ( root_it == roots.end( ) ) continue;

        fs::path path = root_it->second / loc.relative_path;
        if ( !fs::exists( path ) ) continue;

        if ( loc.header_bytes ) {
            if ( !folder_contains_header( path, *loc.header_bytes ) ) continue;
        } else if ( fs::is_empty( path ) ) {
            continue;
        }

        SPDLOG_INFO( "[{}] found: {}", platform_label, loc.game_name );

        Game game;
        game.type = type;
        game.platform_label = std::string( platform_label );
        game.game_name = loc.game_name;
        game.appid = "N/A";
        game.save_paths.push_back( path );
        game.show_parent_path = loc.show_parent_path;
        games.push_back( std::move( game ) );
    }

    return games;
}
