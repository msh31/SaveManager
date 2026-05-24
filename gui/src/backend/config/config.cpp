/*
 *  COPIED FROM SAVEMANAGER -> https://git.marco007.dev/marco/SaveManager/src/branch/main/src/backend/config
 *  AND MODIFIED FOR THIS APP
 */

#include "config.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

CConfig::CConfig( fs::path config_dir ) : m_config_file( config_dir / "config.json" ) {
    try {
        if ( !fs::exists( config_dir ) ) {
            if ( !fs::create_directories( config_dir ) ) {
                throw std::runtime_error( "Failed to create config directory" );
            }
            save( );
        }
        load( );

        // if(!fs::exists(paths::cache_dir())) {
        //     if(!fs::create_directories(paths::cache_dir())) {
        //         SPDLOG_WARN("Failed to create cache directory");
        //     }
        // }
    } catch ( const std::exception& err ) {
        SPDLOG_CRITICAL( "config constructor: {}", err.what( ) );
    }
}

CConfig::~CConfig( ) {
    try {
        save( );
    } catch ( const std::exception& err ) {
        SPDLOG_CRITICAL( "config destructor: {}", err.what( ) );
    }
}

void CConfig::save( ) {
    json data;
    data["dark_mode"] = settings.dark_mode;

    std::ofstream file( m_config_file );
    file << data.dump( 4 );
}

void CConfig::load( ) {
    json data;

    std::ifstream file( m_config_file.c_str( ) );
    if ( !file.is_open( ) ) {
        SPDLOG_ERROR( "Failed to open config!" );
        return;
    }

    try {
        data               = json::parse( file );
        settings.dark_mode = data.value( "dark_mode", true );
    } catch ( json::exception& ex ) {
        SPDLOG_CRITICAL( "config parsing error: {}", ex.what( ) );
    }
}
