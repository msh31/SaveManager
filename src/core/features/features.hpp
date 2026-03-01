#pragma once
#include <zip.h>

#include "core/detection/detection.hpp"

class Features {
public:
    static void backup_game(const Game& game);
    static void restore_backup(const fs::path& name, const Game& selected_game);
    static std::vector<fs::path> get_backups(const Game& game);

private:
    static void create_backup(const fs::path& name, const Game& selected_game);
    static std::string construct_backup_name(const Game& game, const std::string& custom_name = "");
};
