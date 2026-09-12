#include <config/config.hpp>
#include <logger.hpp>
#include <utils/utils.hpp>
#include <utils/network.hpp>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

/*
    TODO LIST:

    1. replace blacklist pre-filling method instead of hardcoding games 
    2. save SFTP information in the OS Keychain (I wanted to use -> https://github.com/hrantzsch/keychain but there is some incompability that needs resolving)
    3. decide if plugins should be repurposed instead of only serving the purpose of being a custom game
*/

CConfig::CConfig( ) {
    try {
        if ( !fs::exists( paths::config_dir() ) ) {
            if ( !fs::create_directories( paths::config_dir( ) ) ) {
                throw std::runtime_error( "[Config] Failed to create config directory" );
            }
        }

        if ( !fs::exists( m_config_file ) ) {
            fs::create_directories( paths::backgrounds_dir( ) );
            fs::create_directories( paths::backup_dir( ) );
            fs::create_directories( paths::plugin_dir( ) ); // 3.
            fs::create_directories( paths::cache_dir( ) );

            m_load_ok = true; //initial creation
            save( );
        }

        init( );

        m_load_ok = load( );
    } catch ( const std::exception& err ) {
        SPDLOG_CRITICAL( "[Config] constructor: {}", err.what( ) );
    }
}

CConfig::~CConfig( ) {
    try {
        SPDLOG_INFO( "[Config] saving before exiting.." );
        save( );
    } catch ( const std::exception& err ) {
        SPDLOG_CRITICAL( "[Config] destructor: {}", err.what( ) );
    }
}

CConfig& CConfig::get( ) {
    static CConfig instance;
    return instance;
}

void CConfig::init( ) {
    if ( !fs::exists( paths::ubi_translations( ) ) ) {
        auto res = Network::download_file( ubi_translation_url.data( ), paths::ubi_translations( ).string( ) );
        if ( res == false ) {
            SPDLOG_ERROR( "[Translations] Failed to download Ubisoft Translations!" );
        }
    }

    if ( !fs::exists( paths::pcgw_manifest( ) ) ) {
        auto res = Network::download_file( pcgw_translation_url.data( ), paths::pcgw_manifest( ).string( ) );
        if ( res == false ) {
            SPDLOG_ERROR( "[Translations] Failed to download SaveManager manifest!" );
        }
    }

    if ( !fs::exists( paths::blacklist( ) ) ) { //1.
        std::ofstream f( paths::blacklist( ) );
        if ( f.is_open( ) ) {
            f << R"(["The Crew Motorfest", "Skull and Bones"])";
            f.close( );
        } else {
            SPDLOG_WARN( "[Blacklist] Failed to open blacklist for writing" );
        }
    }
}

bool CConfig::save( ) {
    if ( !m_load_ok ) {
        SPDLOG_ERROR( "[Config] failed to load!" );
        return false;
    }

    json data;
    data["dark_mode"] = settings.dark_mode;
    data["animated_background"] = settings.animated_background;
    data["startup_update_check"] = settings.startup_update_check;
    data["font_scale"] = settings.font_scale;

    data["show_conflicts"] = d_settings.show_conflicts;
    data["use_savemgr_ignore"] = d_settings.use_savemgr_ignore;
    data["skip_empty_files"] = d_settings.skip_empty_files;

    data["use_bg"] = settings.use_bg;
    data["bg_name"] = settings.bg_name;

    data["window_w"] = settings.window_w;
    data["window_h"] = settings.window_h;

    //2.
    data["dest_addr"] = sftp.dest_addr;
    data["username"] = sftp.username;
    data["password"] = sftp.password;
    data["pubkey"] = sftp.pubkey.string( );
    data["privkey"] = sftp.privkey.string( );
    data["remote_path"] = sftp.remote_path;
    data["key_passphrase"] = sftp.key_passphrase;
    data["auth_pw"] = sftp.auth_pw;
    data["known_hosts"] = sftp.known_hosts;

    auto res = utils::atomic_write( m_config_file, data.dump( 4 ) );
    if ( !res ) {
        SPDLOG_ERROR( "[Config] failed to save!" );
        return false;
    }
    return true;
}

bool CConfig::load( ) {
    json data;

    std::ifstream file( m_config_file.c_str( ) );
    if ( !file.is_open( ) ) {
        SPDLOG_ERROR( "[Config] Failed to open!" );
        return false;
    }

    try {
        data = json::parse( file );

        settings.dark_mode = data.value( "dark_mode", true );
        settings.animated_background = data.value( "animated_background", false );
        settings.startup_update_check = data.value( "startup_update_check", true );
        settings.font_scale = data.value("font_scale", 1.069f);

        settings.use_bg = data.value( "use_bg", false );
        settings.bg_name = data.value( "bg_name", std::string( "" ) );

        d_settings.show_conflicts = data.value( "show_conflicts", false );
        d_settings.use_savemgr_ignore = data.value( "use_savemgr_ignore", false );
        d_settings.skip_empty_files = data.value( "skip_empty_files", false );

        //2.
        sftp.dest_addr = data.value( "dest_addr", std::string( "" ) );
        sftp.username = data.value( "username", std::string( "" ) );
        sftp.password = data.value( "password", std::string( "" ) );
        sftp.remote_path = data.value( "remote_path", std::string( "" ) );
        sftp.pubkey = data.value( "pubkey", fs::path( "" ) );
        sftp.privkey = data.value( "privkey", fs::path( "" ) );
        sftp.key_passphrase = data.value( "key_passphrase", std::string( "" ) );
        sftp.auth_pw = data.value( "auth_pw", false );
        sftp.known_hosts = data.value<std::unordered_map<std::string, std::string>>( "known_hosts", { } );

        settings.window_w = data.value( "window_w", 0 );
        settings.window_h = data.value( "window_h", 0 );
    } catch ( json::exception& ex ) {
        file.close( );
        SPDLOG_CRITICAL( "[Config] parsing error: {}", ex.what( ) );
        m_was_reset = true;

        std::error_code ec;
        fs::rename( m_config_file, m_config_file.string( ) + ".bak", ec );
        if ( ec ) {
            SPDLOG_CRITICAL( "[Config] failed to backup config! this is bad..." );
            return false;
        }

        settings = { };
        return true;
    }
    return true;
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