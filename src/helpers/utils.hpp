#pragma once
#include <zip.h>

#include <ctime>
#include <string>
#include <filesystem>

#include "../detection/detection.hpp"

namespace fs = std::filesystem;

#ifdef _WIN32
#define COLOR_RED ""
#define COLOR_GREEN ""
#define COLOR_BLUE ""
#define COLOR_YELLOW ""
#define COLOR_RESET ""
#else
#define COLOR_RED "\e[0;31m"
#define COLOR_GREEN "\033[1;32m"
#define COLOR_BLUE "\033[1;34m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_RESET "\033[0m"
#endif

#if defined(__linux__) || defined(__APPLE__)
inline fs::path backup_dir = fs::path(std::getenv("HOME"))
           / ".config"
           / "savemanager"
           / "backup";

#elif defined(_WIN32)
inline fs::path backup_dir = fs::path(std::getenv("APPDATA"))
           / "savemanager"
           / "backup";

#else
#error "Unsupported platform"
#endif

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
    if(!fs::exists(backup_dir)) {
        return fs::create_directories(backup_dir);
    }
   
    return true;
}
