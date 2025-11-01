#pragma once
#include "../savemanager.hpp"
#include <fstream>

struct AppConfig {
    std::string selectedProfileID;
};

class Config {
    public:
        AppConfig cfgData;

        Config();
        ~Config();

        void save();
        void load();
    private:
        std::string getConfigPath();
};
