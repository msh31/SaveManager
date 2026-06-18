#include "config/config.hpp"
#include "../utils/translations/steamids.hpp"
#include "../utils/translations/ubi_translations.hpp"

#include "utils/paths.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

CConfig::CConfig( fs::path config_dir ) : config_file( config_dir / "config.json" ) {
    try {
        if ( !fs::exists( config_dir ) ) {
            if ( !fs::create_directories( config_dir ) ) {
                throw std::runtime_error( "Failed to create config directory" );
            }
        }

        if ( !fs::exists( paths::log_dir( ) ) ) {
            if ( !fs::create_directories( paths::log_dir( ) ) ) {
                throw std::runtime_error( "Failed to create log directory" );
            }
        }

        if ( !fs::exists( paths::backup_dir( ) ) ) {
            if ( !fs::create_directories( paths::backup_dir( ) ) ) {
                throw std::runtime_error( "Failed to create backup directory" );
            }
        }

        if ( !fs::exists( paths::plugin_dir( ) ) ) {
            if ( !fs::create_directories( paths::plugin_dir( ) ) ) {
                throw std::runtime_error( "Failed to create plugins directory" );
            }
        }

        if ( !fs::exists( paths::cache_dir( ) ) ) {
            if ( !fs::create_directories( paths::cache_dir( ) ) ) {
                throw std::runtime_error( "Failed to create cache directory" );
            }
        }

        load( );
    } catch ( const std::exception& err ) {
        auto er = std::format( "config constructor: {}", err.what( ) );
        throw std::runtime_error( er );
    }
}

CConfig::~CConfig( ) {
    try {
        save( );
    } catch ( const std::exception& err ) {
        auto er = std::format( "config destructor: {}", err.what( ) );
        SPDLOG_CRITICAL( er );
    }
}

void CConfig::init( ) {
    if ( !fs::exists( paths::ubi_translations( ) ) ) {
        std::ofstream f( paths::ubi_translations( ), std::ios::binary );
        if ( f.is_open( ) ) {
            f.write( reinterpret_cast<const char*>( ubi_translations_json ), ubi_translations_json_len );
        } else {
            SPDLOG_WARN( "Failed to open ubisoft translations for writing!" );
        }
    }

    if ( !fs::exists( paths::steam_appids( ) ) ) {
        std::ofstream f( paths::steam_appids( ), std::ios::binary );
        if ( f.is_open( ) ) {
            f.write( reinterpret_cast<const char*>( steamids_json ), steamids_json_len );
        } else {
            SPDLOG_WARN( "Failed to open steamids for writing" );
        }
    }

    if ( !fs::exists( paths::blacklist( ) ) ) {
        std::ofstream f( paths::blacklist( ) );
        if ( f.is_open( ) ) {
            f << R"(["The Crew Motorfest", "Skull and Bones"])"; // kinda sucks
        } else {
            SPDLOG_WARN( "Failed to open blacklist for writing" );
        }
    }
}

void CConfig::save( ) {
    json data;
    data["dark_mode"] = settings.dark_mode;
    data["animated_background"] = settings.animated_background;

    data["watch_paths"] = settings.watch_paths |
                          std::views::transform( []( const fs::path& p ) { return p.string( ); } ) |
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
    if ( !file.is_open( ) ) {
        throw std::runtime_error( "Failed to open config!" );
    }
    file << data.dump( 4 );
    if ( !file.good( ) ) throw std::runtime_error( "Failed to save config, disk might be full!" );
}

void CConfig::load( ) {
    json data;

    if ( !fs::exists( config_file ) ) {
        save( );
    }

    std::ifstream file( config_file.c_str( ) );
    if ( !file.is_open( ) ) {
        throw std::runtime_error( "Failed to open config!" );
    }

    try {
        data = json::parse( file );

        settings.dark_mode = data.value( "dark_mode", true );
        settings.animated_background = data.value( "animated_background", false );

        if ( data.contains( "watch_paths" ) ) {
            settings.watch_paths = data["watch_paths"] |
                                   std::views::transform( []( const std::string& p ) { return fs::path( p ); } ) |
                                   std::ranges::to<std::vector>( );
        }

        // TODO: improve this by using a keychain on the OS
        sftp.dest_addr = data.value( "dest_addr", std::string( "" ) );
        sftp.username = data.value( "username", std::string( "" ) );
        sftp.password = data.value( "password", std::string( "" ) );
        sftp.remote_path = data.value( "remote_path", std::string( "" ) );
        sftp.pubkey = data.value( "pubkey", fs::path( "" ) );
        sftp.privkey = data.value( "privkey", fs::path( "" ) );

        win_props.x = data.value( "x", -1 );
        win_props.y = data.value( "y", -1 );
        win_props.width = data.value( "width", -1 );
        win_props.height = data.value( "height", -1 );
    } catch ( json::exception& ex ) {
        auto er = std::format( "config parsing error: {}", ex.what( ) );
        throw std::runtime_error( er );
    }
}
