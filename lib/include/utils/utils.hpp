#pragma once
#include "types.hpp"
#include <zip.h>

struct ImFont;

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


// Source - https://stackoverflow.com/a/5253245
// Posted by Blastfurnace, modified by community. See post 'Timeline' for change history
// Retrieved 2026-02-03, License - CC BY-SA 2.5
inline std::string space2underscore(std::string text) {
    std::replace(text.begin(), text.end(), ' ', '_');
    return text;
}

inline std::string sanitize_filename(std::string text) {
    const std::string invalid = "<>:\"/\\|?*";
    std::replace_if(text.begin(), text.end(), [&](char c) {
        return invalid.find(c) != std::string::npos;
    }, '_');
    return text;
}


static std::string_view get_platform_label(PlatformType t) {
    switch(t) {
        case PlatformType::UBISOFT:   return "Ubisoft";
        case PlatformType::ROCKSTAR:  return "Rockstar";
        case PlatformType::UNREAL:    return "Unreal";
        case PlatformType::PSP:       return "PSP";
        case PlatformType::PPSSPP:    return "PPSSPP";
        case PlatformType::MINECRAFT:    return "Minecraft"; //change to launcher?
        case PlatformType::CUSTOM:    return "CUSTOM";
    }
    return "";
}

static std::string_view get_launcher_label(LauncherType t) {
    switch(t) {
        case LauncherType::OFFICIAL:   return "Official";
        case LauncherType::MODRINTH:  return "Modrinth";
        case LauncherType::CURSEFORGE:    return "CurseForge";
        case LauncherType::PRISM:       return "Prism";
        case LauncherType::MULTIMC:    return "MultiMC";
    }
    return "";
}

inline std::string cache_key(const Game& game) {
    if(game.type == PlatformType::MINECRAFT) {
        return "Minecraft";
    }
    return game.game_name;
}
