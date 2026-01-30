#pragma once
#include <zip.h>

#include <ctime>
#include <string>
#include <filesystem>

#include "../detection/detection.hpp"

namespace fs = std::filesystem;

#define COLOR_RED "\e[0;31m"
#define COLOR_GREEN "\033[1;32m"
#define COLOR_BLUE "\033[1;34m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_RESET "\033[0m"

inline std::string construct_backup_name(const Detection::UbiGame& game) {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    char time_buf[20];
    std::strftime(time_buf, sizeof(time_buf), "%Y%m%d_%H%M%S", &tm);

    return "backup_" + game.game_id + "_" + std::string(time_buf) + ".zip";
}

inline std::string print_title()
{
    return R"(
  ██████  ▄▄▄    ██▒   █▓▓█████     ███▄ ▄███▓ ▄▄▄       ███▄    █  ▄▄▄        ▄████ ▓█████  ██▀███  
▒██    ▒ ▒████▄ ▓██░   █▒▓█   ▀    ▓██▒▀█▀ ██▒▒████▄     ██ ▀█   █ ▒████▄     ██▒ ▀█▒▓█   ▀ ▓██ ▒ ██▒
░ ▓██▄   ▒██  ▀█▄▓██  █▒░▒███      ▓██    ▓██░▒██  ▀█▄  ▓██  ▀█ ██▒▒██  ▀█▄  ▒██░▄▄▄░▒███   ▓██ ░▄█ ▒
  ▒   ██▒░██▄▄▄▄██▒██ █░░▒▓█  ▄    ▒██    ▒██ ░██▄▄▄▄██ ▓██▒  ▐▌██▒░██▄▄▄▄██ ░▓█  ██▓▒▓█  ▄ ▒██▀▀█▄  
▒██████▒▒ ▓█   ▓██▒▒▀█░  ░▒████▒   ▒██▒   ░██▒ ▓█   ▓██▒▒██░   ▓██░ ▓█   ▓██▒░▒▓███▀▒░▒████▒░██▓ ▒██▒
▒ ▒▓▒ ▒ ░ ▒▒   ▓▒█░░ ▐░  ░░ ▒░ ░   ░ ▒░   ░  ░ ▒▒   ▓▒█░░ ▒░   ▒ ▒  ▒▒   ▓▒█░ ░▒   ▒ ░░ ▒░ ░░ ▒▓ ░▒▓░
░ ░▒  ░ ░  ▒   ▒▒ ░░ ░░   ░ ░  ░   ░  ░      ░  ▒   ▒▒ ░░ ░░   ░ ▒░  ▒   ▒▒ ░  ░   ░  ░ ░  ░  ░▒ ░ ▒░
░  ░  ░    ░   ▒     ░░     ░      ░      ░     ░   ▒      ░   ░ ░   ░   ▒   ░ ░   ░    ░     ░░   ░ 
      ░        ░  ░   ░     ░  ░          ░         ░  ░         ░       ░  ░      ░    ░  ░   ░     
                     ░                                                                               
    )";
}


inline bool config_exists() {
    std::string home = std::getenv("HOME");
    std::string backupPath = home + "/.config/savemanager/backup";

    if(!fs::exists(backupPath)) {
        return fs::create_directories(backupPath);
    }
   
    return true;
}
