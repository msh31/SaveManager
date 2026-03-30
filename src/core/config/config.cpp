#include "config.hpp"
#include "core/logger/logger.hpp"
#include "core/helpers/translations/steamids.hpp"
#include "core/helpers/translations/ubi_translations.hpp"
#include "../../external/json.hpp"

using json = nlohmann::json;

Config::Config() {
    try {
        if(!fs::exists(paths::backup_dir())) {
            if(!fs::create_directories(paths::backup_dir())) {
                get_logger().error("Failed to create backup directory");
            }
        }

        if(!fs::exists(paths::cache_dir())) {
            if(!fs::create_directories(paths::cache_dir())) {
                get_logger().error("Failed to create cache directory");
            }
        }

        load();
    } catch (const std::exception& err) {
        get_logger().error(err.what());
    }
}

Config::~Config() {
    try {
        save();
    } catch (const std::exception& err) {
        get_logger().error(err.what());
    }
}

bool Config::init() {
    if(!fs::exists(paths::ubi_translations())) {
        std::ofstream f(paths::ubi_translations(), std::ios::binary);
        f.write(reinterpret_cast<const char*>(ubi_translations_json), ubi_translations_json_len);
    }

    if(!fs::exists(paths::steam_appids())) {
        std::ofstream f(paths::steam_appids(), std::ios::binary);
        f.write(reinterpret_cast<const char*>(steamids_json), steamids_json_len);
    }

    if(!fs::exists(paths::blacklist())) {
        std::ofstream f(paths::blacklist());
        f << "[]";
    }

    if(!fs::exists(paths::custom_games())) {
        std::ofstream f(paths::custom_games());
        f << "[]";
    }

    return true;
}

void Config::save() {
    json data;
    data["ubi_enabled"] = settings.ubi_enabled;
    data["rsg_enabled"] = settings.rsg_enabled;
    data["unreal_enabled"] = settings.unreal_enabled;
    data["dark_mode"] = settings.dark_mode;

    data["backup_path"] = settings.backup_path.string();
    data["steam_path"] = settings.steam_path;
    data["lutris_path"] = settings.lutris_path;
    data["heroic_path"] = settings.heroic_path;

    data["dest_addr"] = sftp.dest_addr;
    data["username"] = sftp.username;
    data["password"] = sftp.password;
    data["pubkey"] = sftp.pubkey.string();
    data["privkey"] = sftp.privkey.string();
    data["remote_path"] = sftp.remote_path;

    std::ofstream file(config_file);
    file << data.dump(4);
}

void Config::load() {
    json data;

    if(!fs::exists(config_file)) {
        save();
    }

    std::ifstream file(config_file.c_str());
    if(!file.is_open()) {
        get_logger().error("Failed to open config!");
        return;
    }

    data = json::parse(file);
    settings.backup_path = data.value("backup_path", std::string(""));
    settings.steam_path = data.value("steam_path", std::string(""));
    settings.lutris_path = data.value("lutris_path", std::string(""));
    settings.heroic_path = data.value("heroic_path", std::string(""));

    if (settings.backup_path.empty()) {
        settings.backup_path = paths::backup_dir();
    }
#ifdef __linux__
    if (settings.lutris_path.empty()) {
        settings.lutris_path = paths::lutris_dir().string();
    }
#endif
#ifndef _WIN32
    if (settings.heroic_path.empty()) {
        settings.heroic_path = paths::heroic_dir().string();
    }
#endif // !__WIN32
    settings.ubi_enabled = data.value("ubi_enabled", true);
    settings.rsg_enabled = data.value("rsg_enabled", true);
    settings.unreal_enabled = data.value("unreal_enabled", true);
    settings.dark_mode = data.value("dark_mode", true);

    sftp.dest_addr = data.value("dest_addr", std::string(""));
    sftp.username = data.value("username", std::string(""));
    sftp.password = data.value("password", std::string(""));
    sftp.remote_path = data.value("remote_path", std::string(""));
    sftp.pubkey = data.value("pubkey", fs::path(""));
    sftp.privkey = data.value("privkey", fs::path(""));
}
