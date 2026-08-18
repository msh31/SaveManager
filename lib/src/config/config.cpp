#include "config/config.hpp"
#include "utils/paths.hpp"
#include <network/network.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// TODO: chill on the runtime errors man sheesh
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

bool CConfig::init( ) {
    bool success = true;

    if ( !fs::exists( paths::config_dir( ) ) ) {
        SPDLOG_WARN( "Failed to find config path!" );
        return false;
    }

    if ( !fs::exists( paths::ubi_translations( ) ) ) {
        auto res = Network::download_file( ubi_translation_url, paths::ubi_translations( ).string( ) );
        if ( res == false ) SPDLOG_ERROR( "[Translations] Failed to download Ubisoft Translations!" );
    }

    if ( !fs::exists( paths::pcgw_manifest( ) ) ) {
        auto res = Network::download_file( pcgw_translation_url, paths::pcgw_manifest( ).string( ) );
        if ( res == false ) SPDLOG_ERROR( "[Translations] Failed to download PCGamingWiki manifest!" );
    }

    if ( !fs::exists( paths::blacklist( ) ) ) {
        std::ofstream f( paths::blacklist( ) );
        if ( f.is_open( ) ) {
            f << R"(["The Crew Motorfest", "Skull and Bones"])"; // kinda sucks
            success = success && f.good( );
            f.close( );
        } else {
            SPDLOG_WARN( "Failed to open blacklist for writing" );
            success = false;
        }
    }

    return success;
}

void CConfig::save( ) {
    std::lock_guard<std::recursive_mutex> lock( m_mutex );
    json data;
    data["dark_mode"] = settings.dark_mode;
    data["animated_background"] = settings.animated_background;
    data["startup_update_check"] = settings.startup_update_check;

    data["show_conflicts"] = d_settings.show_conflicts;
    data["use_savemgr_ignore"] = d_settings.use_savemgr_ignore; // TODO
    data["skip_empty_files"] = d_settings.skip_empty_files;

    // data["watch_paths"] = settings.watch_paths |
    //                       std::views::transform( []( const fs::path& p ) { return p.string( ); } ) |
    //                       std::ranges::to<std::vector>( );

    data["dest_addr"] = sftp.dest_addr;
    data["username"] = sftp.username;
    data["password"] = sftp.password;
    data["pubkey"] = sftp.pubkey.string( );
    data["privkey"] = sftp.privkey.string( );
    data["remote_path"] = sftp.remote_path;
    data["key_passphrase"] = sftp.key_passphrase; // plaintext....
    data["auth_pw"] = sftp.auth_pw;
    data["known_hosts"] = sftp.known_hosts;

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
        settings.startup_update_check = data.value( "startup_update_check", true );

        d_settings.show_conflicts = data.value( "show_conflicts", false );
        d_settings.use_savemgr_ignore = data.value( "use_savemgr_ignore", false ); // TODO
        d_settings.skip_empty_files = data.value( "skip_empty_files", false );

        // if ( data.contains( "watch_paths" ) ) {
        //     settings.watch_paths = data["watch_paths"] |
        //                            std::views::transform( []( const std::string& p ) { return fs::path( p ); } ) |
        //                            std::ranges::to<std::vector>( );
        // }

        // TODO: improve this by using a keychain on the OS
        // COMMENT: I wanted to use -> https://github.com/hrantzsch/keychain
        // but it fails to build because of a glib version mismatch
        // and i cant figure out how to fix in the project even tho
        // its not my bug, will need to look into this.
        sftp.dest_addr = data.value( "dest_addr", std::string( "" ) );
        sftp.username = data.value( "username", std::string( "" ) );
        sftp.password = data.value( "password", std::string( "" ) );
        sftp.remote_path = data.value( "remote_path", std::string( "" ) );
        sftp.pubkey = data.value( "pubkey", fs::path( "" ) );
        sftp.privkey = data.value( "privkey", fs::path( "" ) );
        sftp.key_passphrase = data.value( "key_passphrase", std::string( "" ) );
        sftp.auth_pw = data.value( "auth_pw", false );
        sftp.known_hosts = data.value<std::unordered_map<std::string, std::string>>( "known_hosts", { } );

        win_props.x = data.value( "x", -1 );
        win_props.y = data.value( "y", -1 );
        win_props.width = data.value( "width", -1 );
        win_props.height = data.value( "height", -1 );
    } catch ( json::exception& ex ) {
        auto er = std::format( "config parsing error, prevering config and generating a new one: {}", ex.what( ) );
        SPDLOG_CRITICAL( er );
        fs::rename( config_file, config_file.string( ) + ".bak" );
        load( );
    }
}

CConfig::KNOWN_HOST_RESULT CConfig::verify_known_host( const std::string& addr, const std::string& fingerprint ) {
    std::lock_guard<std::recursive_mutex> lock( m_mutex );

    auto it = sftp.known_hosts.find( addr );
    if ( it == sftp.known_hosts.end( ) ) {
        sftp.known_hosts[addr] = fingerprint;
        save( );
        SPDLOG_INFO( "Adding new host {} to known hosts (fingerprint: {})", addr, fingerprint );
        return KNOWN_HOST_RESULT::NEW;
    } else if ( it->second != fingerprint ) {
        SPDLOG_ERROR(
            "Host key verification failed for {}! Expected {}, got {}. The remote host key may have "
            "changed, or this may be a man-in-the-middle attack.",
            addr, it->second, fingerprint );
        return KNOWN_HOST_RESULT::MISMATCH;
    }

    return KNOWN_HOST_RESULT::MATCH;
}
