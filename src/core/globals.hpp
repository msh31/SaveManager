#pragma once
#include <filesystem>
#include "imgui.h"

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
