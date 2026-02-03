#pragma once
#include <zip.h>
#include <curl/curl.h>

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
           / "backups";

inline fs::path config_dir = fs::path(std::getenv("HOME"))
           / ".config"
           / "savemanager";
#elif defined(_WIN32)
inline fs::path backup_dir = fs::path(std::getenv("APPDATA"))
           / "savemanager"
           / "backups";

inline fs::path config_dir = fs::path(std::getenv("APPDATA"))
           / "savemanager";
#else
#error "Unsupported platform"
#endif

inline std::string construct_backup_name(const Game& game) {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    char time_buf[20];
    std::strftime(time_buf, sizeof(time_buf), "%Y%m%d_%H%M%S", &tm);

    return "backup_" + game.game_name + "_" + std::string(time_buf) + ".zip";
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

inline size_t write_callback(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    return fwrite(ptr, size, nmemb, stream);
}

inline bool download_file(const std::string& url, const std::string& output_path) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;
    
    FILE* fp = fopen(output_path.c_str(), "wb");
    if (!fp) { curl_easy_cleanup(curl); return false; }
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    
    CURLcode res = curl_easy_perform(curl);
    
    fclose(fp);
    curl_easy_cleanup(curl);
    
    return res == CURLE_OK;
}

inline bool download_ubi_translations() {
    fs::path output_path = config_dir / "gameids.json";
    return download_file(
        "https://git.marco007.dev/marco/Ubisoft-Game-Ids/raw/branch/master/gameids.json", 
        output_path.string()
    );
}

inline void wait_for_key() {
    std::cout << "\nPress any key to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}
