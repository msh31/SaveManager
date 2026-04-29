#pragma once

#define APP_NAME "SaveManager"
#define APP_AUTHOR "marco007"
#define APP_VERSION "v1.5.4"

#define MODE_CREATE_ARCHIVE (ZIP_CREATE | ZIP_TRUNCATE)
#define MODE_EXTRACT_ARCHIVE 0

constexpr int MIN_RES_W = 1280;
constexpr int MIN_RES_H = 720;
constexpr int MAX_RES_W = 5120;
constexpr int MAX_RES_H = 2880;

constexpr int DEF_RES_W = 1600;
constexpr int DEF_RES_H = 900;

constexpr std::array<std::string_view, 10> extension_blocklist{
    ".dat", ".bin", ".upload", ".bak", ".cfg", ".log", ".tmp", ".ini", ".set", ".txt"
};
