#pragma once
#include <utils/paths.hpp>

class CConfig {
    public:
        CConfig( fs::path config_dir = paths::config_dir( ) );
        ~CConfig( );

        void init( );
        void save( );

        enum class KNOWN_HOST_RESULT { NEW, MATCH, MISMATCH };
        KNOWN_HOST_RESULT verify_known_host( const std::string& addr, const std::string& fingerprint );

        
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

        struct AppConfig {
                bool dark_mode = true;
                bool animated_background = false;
                bool startup_update_check = true;

                bool use_bg = false;
                std::string bg_name = { };

                // 0 means "unset" - window_manager computes a default from the primary monitor
                int window_w = 0;
                int window_h = 0;
        };

        AppConfig settings;
        DetectionSettings d_settings;
        SFTPConfig sftp;

    private:
        fs::path m_config_file = paths::config_dir( ) / "config.json";

        bool load( );
        bool m_load_ok = false;

        std::recursive_mutex m_mutex;

        std::string_view ubi_translation_url =
            "https://raw.githubusercontent.com/msh31/SaveManager/refs/heads/dev/data/ubi_translations.json";
        std::string_view pcgw_translation_url =
            "https://raw.githubusercontent.com/msh31/savemanager-manifest/refs/heads/main/data/manifest.json";
};
