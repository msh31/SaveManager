#include "plugin/plugin.hpp"
#include "utils/paths.hpp"
#include "utils/steam/steam.hpp"

CPlugin::CPlugin( std::filesystem::path path ) {
    SPDLOG_INFO( "Loaded plugin: {}", path.filename( ).string( ) );
    m_lua.open_libraries( sol::lib::base, sol::lib::string, sol::lib::table ); // allow print, type, tostring etc

    m_lua["print"] = []( std::string msg ) { SPDLOG_INFO( "[lua] {}", msg ); };

    m_lua.set_function( "is_linux", []( ) {
#if defined( __linux__ )
        return true;
#endif
        return false;
    } );

    m_lua.set_function( "is_macos", []( ) {
#if defined( __APPLE__ )
        return true;
#endif
        return false;
    } );

    m_lua.set_function( "is_windows", []( ) {
#if defined( _WIN32 )
        return true;
#endif
        return false;
    } );

    m_lua.set_function( "path_exists", []( const std::string& p ) { return fs::exists( p ); } );

    m_lua.set_function( "steam_library_paths", []( ) {
        auto libs = SteamHelper::get_library_folders( );
        std::vector<std::string> result;
        for ( const auto& p : libs )
            result.push_back( p.string( ) );
        return result;
    } );

    m_lua.set_function( "home_dir", []( ) { return paths::home_dir( ).string( ); } );

    m_lua.set_function( "documents_dir", []( ) { return paths::documents_dir( ).string( ); } );

    m_lua.set_function( "list_dir", [this]( const std::string& path ) {
        if ( !fs::is_directory( path ) ) return sol::table( );

        sol::table table = m_lua.create_table( );
        int index = 1;

        try {
            for ( const auto& entry : fs::directory_iterator( path ) ) {
                table[index++] = entry.path( ).string( );
            }
        } catch ( const fs::filesystem_error& e ) {
            SPDLOG_WARN( "list_dir: {}", e.what( ) );
        }
        return table;
    } );

    try {
        m_lua.script_file( path.string( ) ); // load and run the script
    } catch ( sol::error& ex ) {
        SPDLOG_ERROR( "LUA error: failed to load script '{}': {}", path.string( ), ex.what( ) );
        return;
    }

    if ( m_lua["config"].valid( ) ) {
        m_show_parent_path = m_lua["config"]["show_parent_path"].get_or( false );
    }
    // SPDLOG_DEBUG("config valid: {}", m_lua["config"].valid());
    // SPDLOG_DEBUG("show_parent_path valid: {}", m_lua["config"]["show_parent_path"].valid());
}

std::vector<Game> CPlugin::find_saves( ) {
    std::vector<Game> games;

    sol::protected_function fn = m_lua["find_saves"];
    if ( !fn.valid( ) ) {
        SPDLOG_ERROR( "Lua plugin missing find_saves function" );
        return { };
    }

    auto result = fn( );
    if ( !result.valid( ) ) {
        sol::error err = result;
        SPDLOG_ERROR( "Lua error: {}", err.what( ) );
        return { };
    }

    sol::table table = result.get<sol::table>( );
    for ( auto& [key, val] : table ) {
        sol::table entry = val.as<sol::table>( );
        std::string game_name = entry["game_name"].get_or<std::string>( "" );
        std::string appid = entry["appid"].get_or<std::string>( "" );
        std::string save_path = entry["save_path"].get_or<std::string>( "" );

        if ( game_name.empty( ) || save_path.empty( ) ) {
            SPDLOG_WARN( "Lua plugin returned entry missing game_name or save_path, skipping" );
            continue;
        }

        Game g;
        g.type = PlatformType::CUSTOM;
        g.game_name = std::move( game_name );
        g.appid = std::move( appid );
        g.save_paths.push_back( save_path ); //?
        g.show_parent_path = m_show_parent_path;
        games.emplace_back( g );
    }
    return games;
}
