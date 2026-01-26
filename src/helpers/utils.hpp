#pragma once
#include <zip.h>
#include <ctime>
#include <string>
#include "../detection/detection.hpp"

inline std::string construct_backup_name(const Detection::UbiGame& game) {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    char time_buf[20];
    std::strftime(time_buf, sizeof(time_buf), "%Y%m%d_%H%M%S", &tm);

    return "backup_" + game.game_id + "_" + std::string(time_buf) + ".zip";
}
