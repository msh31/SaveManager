#pragma once
#include <zip.h>

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
