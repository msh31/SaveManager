#pragma once
#include <utils/paths.hpp>

class CConfig {
    public:
    CConfig( fs::path config_dir = paths::config_dir( ) );
    ~CConfig( );
    void save( );

    struct AppConfig {
        bool dark_mode = true;
    };

    AppConfig settings;

    private:
    fs::path m_config_file = paths::config_dir( ) / "config.json";

    void load( );
};
