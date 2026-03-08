#pragma once
#include <zip.h>
#include <string>
#include <algorithm>

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
