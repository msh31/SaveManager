#pragma once
#include <filesystem>
#include "imgui.h"

constexpr std::string_view APP_VERSION = "v1.3.0";

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

