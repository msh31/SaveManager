#include "config.hpp"

Config::Config() {
    load();
}

Config::~Config() {
    save();
}

void Config::save() {
    logger log;
    try {
        json j;
        j["selectedProfileID"] = cfgData.selectedProfileID;

        std::ofstream file(getConfigPath());
        if (!file.is_open()) {
            log.error("Could not open config.json for writing!");
            return;
        }

        file << j.dump(4);
    } catch (const std::exception& e) {
        log.error("Failed to save config: " + std::string(e.what()));
    }
}

void Config::load() {
    logger log;

    fs::create_directories("./data");

    if (fs::exists(getConfigPath())) {
        std::ifstream configFile(getConfigPath());

        if (!configFile.is_open()) {
            log.error("Could not open config.json!");
            cfgData.selectedProfileID = "";
            return;
        }

        try {
            json configData = json::parse(configFile);
            cfgData.selectedProfileID = configData["selectedProfileID"];
        } catch (const json::exception& e) {
            log.error("Failed to parse config: " + std::string(e.what()));
            cfgData.selectedProfileID = "";
        }
    } else {
        cfgData.selectedProfileID = "";
        save();
    }
}

std::string Config::getConfigPath() {
    return "./data/config.json";
}
