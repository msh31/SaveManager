#include "config.hpp"

Config::Config() {
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
        //TODO: create new json and write the default fields to it
        cfgData.selectedProfileID = "";
        save();
    }
}

Config::~Config() {
    save();
}

void Config::save() {
    //TODO: write changes from the struct into the config,
}

std::string Config::getConfigPath() {
    return "./data/config.json";
}
