#pragma once
#include "utils/paths.hpp"

class CConfig {
    public:
        CConfig( fs::path config_dir = paths::config_dir( ) );
        ~CConfig( );
        bool init( );
        void save( );

        enum class KNOWN_HOST_RESULT { NEW, MATCH, MISMATCH };
        KNOWN_HOST_RESULT verify_known_host( const std::string& addr, const std::string& fingerprint );

        struct WindowProperties {
                int x = -1;
                int y = -1;
                int width = -1;
                int height = -1;
        };

        struct AppConfig {
                bool dark_mode = true;
                bool animated_background = false;
                bool startup_update_check = true;

                // std::vector<fs::path> watch_paths;
        };

        struct DetectionSettings {
                bool show_conflicts = false;
                bool use_savemgr_ignore = false; // TODO
                bool skip_empty_files = true;
        };

        struct SFTPConfig {
                std::string dest_addr;
                std::string username;
                std::string password;
                std::string remote_path;
                fs::path pubkey;
                fs::path privkey;
                std::string key_passphrase;
                bool auth_pw = true;
                std::unordered_map<std::string, std::string> known_hosts = { }; // addr, fingerprint
        };

        AppConfig settings;
        DetectionSettings d_settings;
        SFTPConfig sftp;
        WindowProperties win_props;

    private:
        void load( );

        fs::path config_file = paths::config_dir( ) / "config.json";

        std::recursive_mutex m_mutex;
};
