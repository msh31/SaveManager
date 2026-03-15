#pragma once
#include "core/helpers/utils.hpp"
#include "core/logger/logger.hpp"
#include "core/detection/detection.hpp"
#include "core/helpers/zip_archive/zip_archive.hpp"

class Features {
public:
    static void backup_game(const Game& game, Config& config);
    static void restore_backup(const fs::path& name, const Game& selected_game);
    static std::vector<fs::path> get_backups(const Game& game, Config& config);

private:
    static void create_backup(const fs::path& name, const Game& selected_game);
    static std::string construct_backup_name(const Game& game, const std::string& custom_name = "");
};
