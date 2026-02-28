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
