#include <config/config.hpp>
#include <logger.hpp>
#include <utils/utils.hpp>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

CConfig::CConfig( fs::path config_dir ) : m_config_file( config_dir / "config.json" ) {
    try {
        if ( !fs::exists( config_dir ) ) {
            if ( !fs::create_directories( config_dir ) ) {
                throw std::runtime_error( "Failed to create config directory" );
            }
            m_load_ok = true; //initial creation
            save( );
        }

        fs::create_directories( paths::backgrounds_dir( ) );

        m_load_ok = load( );
    } catch ( const std::exception& err ) {
        SPDLOG_CRITICAL( "config constructor: {}", err.what( ) );
    }
}

CConfig::~CConfig( ) {
    try {
        SPDLOG_INFO( "[Config] saving config before exiting.." );
        save( );
    } catch ( const std::exception& err ) {
        SPDLOG_CRITICAL( "config destructor: {}", err.what( ) );
    }
}

void CConfig::save( ) {
    if ( !m_load_ok ) return;
    json data;
    data["dark_mode"] = settings.dark_mode;
    data["use_bg"] = settings.use_bg;
    data["bg_name"] = settings.bg_name;

    data["window_w"] = settings.window_w;
    data["window_h"] = settings.window_h;

    bool res = utils::atomic_write( m_config_file, data.dump( 4 ) );
    if ( !res ) SPDLOG_ERROR( "[Config] Failed to save config!" );
}

bool CConfig::load( ) {
    json data;
    bool load_ok = false;

    std::ifstream file( m_config_file.c_str( ) );
    if ( !file.is_open( ) ) {
        SPDLOG_ERROR( "Failed to open config!" );
        return load_ok;
    }

    try {
        data = json::parse( file );
        if ( data.empty() ) return load_ok;

        settings.dark_mode = data.value( "dark_mode", true );
        settings.use_bg = data.value( "use_bg", false );
        settings.bg_name = data.value( "bg_name", std::string( "" ) );

        settings.window_w = data.value( "window_w", 0 );
        settings.window_h = data.value( "window_h", 0 );
        load_ok = true;
    } catch ( json::exception& ex ) {
        SPDLOG_CRITICAL( "config parsing error: {}", ex.what( ) );
        return load_ok;
    }
    return load_ok;
}
