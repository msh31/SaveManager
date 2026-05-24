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

    m_lua.set_function( "path_exists", []( const std::string &p ) { return fs::exists( p ); } );

    m_lua.set_function( "steam_library_paths", []( ) {
        auto libs = SteamHelper::get_library_folders( );
        std::vector<std::string> result;
        for ( const auto &p : libs )
            result.push_back( p.string( ) );
        return result;
    } );

    m_lua.set_function( "home_dir", []( ) { return paths::home_dir( ).string( ); } );

    m_lua.set_function( "documents_dir", []( ) { return paths::documents_dir( ).string( ); } );

    m_lua.set_function( "list_dir", [this]( const std::string &path ) {
        if ( !fs::is_directory( path ) ) return sol::table( );

        sol::table table = m_lua.create_table( );
        int index = 1; // lua

        for ( const auto &entry : fs::directory_iterator( path ) ) {
            table[index] = entry.path( ).string( );
            index++;
        }

        return table;
    } );

    m_lua.script_file( path.string( ) ); // load and run the script

    if ( m_lua["config"].valid( ) ) {
        m_show_parent_path = m_lua["config"]["show_parent_path"].get_or( false );
    }
    // SPDLOG_DEBUG("config valid: {}", m_lua["config"].valid());
    // SPDLOG_DEBUG("show_parent_path valid: {}", m_lua["config"]["show_parent_path"].valid());
}

std::vector<Game> CPlugin::find_saves( ) {
    sol::protected_function fn = m_lua["find_saves"];
    auto result = fn( );
    std::vector<Game> games;

    if ( !result.valid( ) ) {
        sol::error err = result;
        SPDLOG_ERROR( "Lua error: {}", err.what( ) );
        return { };
    }

    sol::table table = result.get<sol::table>( );
    for ( auto &[key, val] : table ) {
        sol::table entry = val.as<sol::table>( );
        Game g;
        g.type = PlatformType::CUSTOM;
        g.game_name = entry["game_name"].get<std::string>( );
        g.appid = entry["appid"].get<std::string>( );
        g.save_path = entry["save_path"].get<std::string>( );
        g.show_parent_path = m_show_parent_path;
        games.emplace_back( g );
    }
    return games;
}
