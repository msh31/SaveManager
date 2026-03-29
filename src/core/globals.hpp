#pragma once
#include <filesystem>
#include "imgui.h"

#define APP_NAME "SaveManager" 
#define APP_AUTHOR "marco007" 
#define APP_VERSION "1.4.0" 

constexpr int MIN_RES_W = 1280;
constexpr int MIN_RES_H = 720;
constexpr int MAX_RES_W = 5120;
constexpr int MAX_RES_H = 2880;

constexpr int DEF_RES_W = 1600;
constexpr int DEF_RES_H = 900;

struct Fonts {
    ImFont* regular;
    ImFont* medium;
    ImFont* bold;

    ImFont* title;
    ImFont* header;
};

struct TabState {
    std::vector<std::filesystem::path> backups;
    std::vector<bool> selected_backups;
    int selected_game_idx = 0;
    int selected_backup_idx = 0;
};


struct RemoteEntry {
    std::string name;
    bool is_directory;
};

