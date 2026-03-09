#pragma once
#include <filesystem>

namespace fs = std::filesystem;
#if defined(__linux__) || defined(__APPLE__)
inline fs::path backup_dir = fs::path(std::getenv("HOME"))
           / ".config"
           / "savemanager"
           / "backups";

inline fs::path config_dir = fs::path(std::getenv("HOME"))
           / ".config"
           / "savemanager";
inline fs::path cache_dir = fs::path(std::getenv("HOME"))
           / ".config"
           / "savemanager"
           / "cache";

inline fs::path lutris_dir = fs::path(std::getenv("HOME"))
           / "Games";
#elif defined(_WIN32)
inline fs::path backup_dir = fs::path(std::getenv("APPDATA"))
           / "savemanager"
           / "backups";

inline fs::path config_dir = fs::path(std::getenv("APPDATA"))
           / "savemanager";
inline fs::path cache_dir = fs::path(std::getenv("APPDATA"))
           / "savemanager"
           / "cache";
inline fs::path documents_dir = fs::path(std::getenv("USERPROFILE"))
           / "Documents";
#else
#error "Unsupported platform"
#endif


inline fs::path ubi_translations = config_dir / "ubi_translations.json";
inline fs::path rsg_translations = config_dir / "rsg_translations.json";
inline fs::path steam_appids = config_dir / "steamids.json";
