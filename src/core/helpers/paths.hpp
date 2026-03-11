#pragma once
#include <filesystem>

namespace fs = std::filesystem;

namespace paths {
inline fs::path home_dir() {
    const char* home;
#if defined(__linux__) || defined(__APPLE__)
    home = std::getenv("HOME");
#elif defined(_WIN32)
    home = std::getenv("APPDATA");
#endif
    if (!home) throw std::runtime_error("HOME/APPDATA not set, how did you manage to do this?");
    return fs::path(home);
}

inline fs::path config_dir() {
#if defined(__linux__) || defined(__APPLE__)
    return home_dir() / ".config" / "savemanager";
#elif defined(_WIN32)
    return home_dir() / "savemanager";
#endif
}
inline fs::path backup_dir() {
    return config_dir() / "backups";
}
inline fs::path cache_dir() {
    return config_dir() / "cache";
}

#if defined(__linux__) || defined(__APPLE__)
inline fs::path lutris_dir() {
    return home_dir() / "Games";
}
#endif


#if defined(_WIN32)
inline fs::path documents_dir() {
    const char* userprofile = std::getenv("USERPROFILE");
    if (!userprofile) throw std::runtime_error("USERPROFILE not set, how did you manage to do this?");
    return userprofile;
}
#endif

inline fs::path ubi_translations() { return config_dir() / "ubi_translations.json"; }
inline fs::path rsg_translations() { return config_dir() / "rsg_translations.json"; }
inline fs::path steam_appids() { return config_dir() / "steamids.json"; }
};
