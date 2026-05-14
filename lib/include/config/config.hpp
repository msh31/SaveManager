#pragma once
#include "utils/paths.hpp"

class Config {
public:
    Config(fs::path config_dir = paths::config_dir());
    ~Config();
    bool init();
    void save();

    struct WindowProperties {
        int x = -1;
        int y = -1;
        int width = -1;
        int height = -1;
    };

    struct AppConfig {
        bool ubi_enabled = true;
        bool rsg_enabled = true;
        bool unreal_enabled = true;
        bool dark_mode = true;
        bool animated_background = false;
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
    };

    AppConfig settings;
    SFTPConfig sftp;
    WindowProperties win_props;

private:
    void load();

    fs::path config_file = paths::config_dir() / "config.json";
};
