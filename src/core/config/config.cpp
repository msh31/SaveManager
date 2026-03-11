#include "config.hpp"
#include "core/helpers/paths.hpp"
#include "core/network/network.hpp"
#include "core/logger/logger.hpp"
#include "../../external/json.hpp"

#include <filesystem>
#include <fstream>

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
        get_logger().info("ubi_translations.json not found, downloading...");
        if(!Network::download_file("https://git.marco007.dev/marco/smdata/raw/branch/main/ubi_translations.json", paths::ubi_translations().string())) {
            get_logger().error("Failed to download Ubisoft translations");
            return false;
        }
    }

    if(!fs::exists(paths::steam_appids())) {
        get_logger().info("steamids.json was not found, downloading...");
        if(!Network::download_file("https://git.marco007.dev/marco/smdata/raw/branch/main/steamids.json", paths::steam_appids().string())) {
            get_logger().error("Failed to download Steam ID data");
            return false;
        }
    }

    return true;
}

void Config::save() {
    json data;
    data["backup_path"] = settings.backup_path.string();
    data["ubi_enabled"] = settings.ubi_enabled;
    data["rsg_enabled"] = settings.rsg_enabled;
    data["steam_path"] = settings.steam_path;
    data["lutris_path"] = settings.lutris_path;

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

    if (settings.backup_path.empty()) {
        settings.backup_path = paths::backup_dir();
    }
    if (settings.lutris_path.empty()) {
#ifdef __linux__
        settings.lutris_path = paths::lutris_dir().string();
#endif
    }

    settings.ubi_enabled = data.value("ubi_enabled", true);
    settings.rsg_enabled = data.value("rsg_enabled", true);
}
