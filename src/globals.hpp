#pragma once
#include "imgui.h"

//this entire thing is a it of a mess 

namespace fs = std::filesystem;

#define APP_NAME "SaveManager" 
#define APP_AUTHOR "marco007" 
#define APP_VERSION "v1.5.0" 

constexpr int MIN_RES_W = 1280;
constexpr int MIN_RES_H = 720;
constexpr int MAX_RES_W = 5120;
constexpr int MAX_RES_H = 2880;

constexpr int DEF_RES_W = 1600;
constexpr int DEF_RES_H = 900;

struct Fonts {
    ImFont* regular;
    ImFont* medium;
    ImFont* small_font;
    ImFont* bold;

    ImFont* title;
    ImFont* header;
};

//TODO: refactor transfer tab so this isnt needed
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

enum class DetectionError {
    PathNotFound,
    PermissionDenied,
    NoSavesFound,
};

enum class PlatformType {
    UBISOFT = 1,
    ROCKSTAR,
    UNREAL,
    PSP,
    PPSSPP,
    CUSTOM
};

struct Game {
    PlatformType type;
    std::string appid;
    std::optional<std::string> game_id; 
    std::string game_name;
    std::filesystem::path save_path;
};


//apple clang doesnt support c++23 views as of apr 2026 
template<typename Range, typename Fn> void enumerate(Range& range, Fn fn) {
#ifdef __APPLE__
    int i = 0;
    for (auto& r : range) {
        fn(i, r);
        ++i;
    }
#else
    for (auto [i, element] : std::views::enumerate(range)) {
        fn(i, element);
    }
#endif
}
