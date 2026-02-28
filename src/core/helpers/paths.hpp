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
#elif defined(_WIN32)
inline fs::path backup_dir = fs::path(std::getenv("APPDATA"))
           / "savemanager"
           / "backups";

inline fs::path config_dir = fs::path(std::getenv("APPDATA"))
           / "savemanager";
inline fs::path cache_dir = fs::path(std::getenv("HOME"))
           / ".config"
           / "savemanager"
           / "cache";
#else
#error "Unsupported platform"
#endif
