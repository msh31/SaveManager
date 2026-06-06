#include "plugin/plugin.hpp"
#include "utils/paths.hpp"
#include "utils/steam/steam.hpp"

Plugin::Plugin( std::filesystem::path path ) {
    SPDLOG_INFO( "Loaded plugin: {}", path.filename( ).string( ) );
    lua.open_libraries( sol::lib::base, sol::lib::string, sol::lib::table ); // allow print, type, tostring etc

    lua["print"] = []( std::string msg ) { SPDLOG_INFO( "[lua] {}", msg ); };

    lua.set_function( "is_linux", []( ) {
#if defined( __linux__ )
        return true;
#endif
        return false;
    } );

    lua.set_function( "is_macos", []( ) {
#if defined( __APPLE__ )
        return true;
#endif
        return false;
    } );

    lua.set_function( "is_windows", []( ) {
#if defined( _WIN32 )
        return true;
#endif
        return false;
    } );

    lua.set_function( "path_exists", []( const std::string &p ) { return fs::exists( p ); } );

    lua.set_function( "steam_library_paths", []( ) {
        auto libs = SteamHelper::get_library_folders( );
        std::vector<std::string> result;
        for ( const auto &p : libs )
            result.push_back( p.string( ) );
        return result;
    } );

    lua.set_function( "home_dir", []( ) { return paths::home_dir( ).string( ); } );

    lua.set_function( "documents_dir", []( ) { return paths::documents_dir( ).string( ); } );

    lua.set_function( "list_dir", [this]( const std::string &path ) {
        if ( !fs::is_directory( path ) ) return sol::table( );

        sol::table table = lua.create_table( );
        int index = 1;

        try {
            for ( const auto &entry : fs::directory_iterator( path ) ) {
                table[index++] = entry.path( ).string( );
            }
        } catch ( const fs::filesystem_error &e ) {
            SPDLOG_WARN( "list_dir: {}", e.what( ) );
        }
        return table;
    } );

    lua.script_file( path.string( ) ); // load and run the script

    if ( lua["config"].valid( ) ) {
        show_parent_path = lua["config"]["show_parent_path"].get_or( false );
    }
    // SPDLOG_DEBUG("config valid: {}", lua["config"].valid());
    // SPDLOG_DEBUG("show_parent_path valid: {}", lua["config"]["show_parent_path"].valid());
}

std::vector<Game> Plugin::find_saves( ) {
    sol::protected_function fn = lua["find_saves"];
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
        g.show_parent_path = show_parent_path;
        games.emplace_back( g );
    }
    return games;
}
