#include "config/config.hpp"
#include "utils/paths.hpp"
#include "utils/translations/steamids.hpp"
#include "utils/translations/ubi_translations.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

Config::Config( fs::path config_dir ) : config_file( config_dir / "config.json" ) {
    try {
        if ( !fs::exists( paths::log_dir( ) ) ) {
            if ( !fs::create_directories( paths::log_dir( ) ) ) {
                SPDLOG_ERROR( "Failed to create log directory" );
            }
        }

        if ( !fs::exists( config_dir ) ) {
            if ( !fs::create_directories( config_dir ) ) {
                SPDLOG_ERROR( "Failed to create backup directory" );
            }
        }

        if ( !fs::exists( paths::backup_dir( ) ) ) {
            if ( !fs::create_directories( paths::backup_dir( ) ) ) {
                SPDLOG_ERROR( "Failed to create backup directory" );
            }
        }

        if ( !fs::exists( paths::plugin_dir( ) ) ) {
            if ( !fs::create_directories( paths::plugin_dir( ) ) ) {
                SPDLOG_ERROR( "Failed to create plugins directory" );
            }
        }

        // if(!fs::exists(paths::cache_dir())) {
        //     if(!fs::create_directories(paths::cache_dir())) {
        //         SPDLOG_ERROR("Failed to create cache directory");
        //     }
        // }

        load( );
    } catch ( const std::exception &err ) {
        SPDLOG_ERROR( "config constructor: {}", err.what( ) );
    }
}

Config::~Config( ) {
    try {
        save( );
    } catch ( const std::exception &err ) {
        SPDLOG_ERROR( "config destructor: {}", err.what( ) );
    }
}

bool Config::init( ) {
    if ( !fs::exists( paths::ubi_translations( ) ) ) {
        std::ofstream f( paths::ubi_translations( ), std::ios::binary );
        f.write( reinterpret_cast<const char *>( ubi_translations_json ), ubi_translations_json_len );
    }

    if ( !fs::exists( paths::steam_appids( ) ) ) {
        std::ofstream f( paths::steam_appids( ), std::ios::binary );
        f.write( reinterpret_cast<const char *>( steamids_json ), steamids_json_len );
    }

    if ( !fs::exists( paths::blacklist( ) ) ) {
        std::ofstream f( paths::blacklist( ) );
        f << R"(["The Crew Motorfest"])"; // kinda sucks
    }

    // if(!fs::exists(paths::custom_games())) {
    //     std::ofstream f(paths::custom_games());
    //     f << "[]";
    // }

    return true;
}

void Config::save( ) {
    json data;
    data["ubi_enabled"] = settings.ubi_enabled;
    data["rsg_enabled"] = settings.rsg_enabled;
    data["unreal_enabled"] = settings.unreal_enabled;

    data["dark_mode"] = settings.dark_mode;
    data["animated_background"] = settings.animated_background;

    data["watch_paths"] = settings.watch_paths |
                          std::views::transform( []( const fs::path &p ) { return p.string( ); } ) |
                          std::ranges::to<std::vector>( );

    data["dest_addr"] = sftp.dest_addr;
    data["username"] = sftp.username;
    data["password"] = sftp.password;
    data["pubkey"] = sftp.pubkey.string( );
    data["privkey"] = sftp.privkey.string( );
    data["remote_path"] = sftp.remote_path;

    data["x"] = win_props.x;
    data["y"] = win_props.y;
    data["width"] = win_props.width;
    data["height"] = win_props.height;

    std::ofstream file( config_file );
    file << data.dump( 4 );
}

void Config::load( ) {
    json data;

    if ( !fs::exists( config_file ) ) {
        save( );
    }

    std::ifstream file( config_file.c_str( ) );
    if ( !file.is_open( ) ) {
        SPDLOG_ERROR( "Failed to open config!" );
        return;
    }

    try {
        data = json::parse( file );
        settings.ubi_enabled = data.value( "ubi_enabled", true );
        settings.rsg_enabled = data.value( "rsg_enabled", true );
        settings.unreal_enabled = data.value( "unreal_enabled", true );

        settings.dark_mode = data.value( "dark_mode", true );
        settings.animated_background = data.value( "animated_background", false );

        if ( data.contains( "watch_paths" ) ) {
            settings.watch_paths = data["watch_paths"] |
                                   std::views::transform( []( const std::string &p ) { return fs::path( p ); } ) |
                                   std::ranges::to<std::vector>( );
        }

        sftp.dest_addr = data.value( "dest_addr", std::string( "" ) );
        sftp.username = data.value( "username", std::string( "" ) );
        sftp.password = data.value( "password", std::string( "" ) );
        sftp.remote_path = data.value( "remote_path", std::string( "" ) );
        sftp.pubkey = data.value( "pubkey", fs::path( "" ) );
        sftp.privkey = data.value( "privkey", fs::path( "" ) );

        win_props.x = data["x"];
        win_props.y = data["y"];
        win_props.width = data["width"];
        win_props.height = data["height"];
    } catch ( json::exception &ex ) {
        SPDLOG_ERROR( "config parsing error: {}", ex.what( ) );
    }
}
