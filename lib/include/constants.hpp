#pragma once

constexpr std::string_view APP_NAME = "SaveManager";
constexpr std::string_view APP_AUTHOR = "marco007";
constexpr std::string_view APP_VERSION = "v1.8.1";

#define MODE_CREATE_ARCHIVE ( ZIP_CREATE | ZIP_TRUNCATE )
#define MODE_EXTRACT_ARCHIVE 0

constexpr int MIN_RES_W = 1280;
constexpr int MIN_RES_H = 720;
constexpr int MAX_RES_W = 5120;
constexpr int MAX_RES_H = 2880;

constexpr int DEF_RES_W = 1600;
constexpr int DEF_RES_H = 900;

constexpr std::string_view ubi_translation_url =
    "https://raw.githubusercontent.com/msh31/SaveManager/refs/heads/dev/data/ubi_translations.json";
constexpr std::string_view steam_translation_url =
    "https://raw.githubusercontent.com/msh31/SaveManager/refs/heads/dev/data/steamids.json";

static const std::unordered_set<std::string_view> extension_blocklist{
    ".dat", ".bin",  ".upload", ".bak",  ".cfg",  ".log", ".tmp", ".ini",    ".set",
    ".txt", ".lock", ".lck",    ".part", ".temp", ".swp", ".swo", ".journal" };

static const std::unordered_set<std::string_view> g_extension_blocklist{ ".png", ".jpg", ".jpeg", ".webp", ".bmp" };
