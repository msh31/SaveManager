#pragma once
#include "core/helpers/paths.hpp"
#include <string>
#include <filesystem>
namespace fs = std::filesystem;

class Config {
public:
    Config();
    ~Config();
    bool init();
    void save();

    struct AppConfig {
        fs::path backup_path;
        bool ubi_enabled = true;
        bool rsg_enabled = true;
        bool unreal_enabled = true;
        std::string steam_path;
        std::string lutris_path;
        std::string heroic_path;
    };

    struct SFTPConfig {
        std::string dest_addr;
        std::string username;
        std::string password;
        std::string remote_path;
        fs::path pubkey;
        fs::path privkey;
    };

    AppConfig settings;
    SFTPConfig sftp;

private:
    void load();

    fs::path config_file = paths::config_dir() / "config.json";
};
