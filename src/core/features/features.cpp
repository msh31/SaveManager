#include "features.hpp"
#include "../../core/backup/backup.hpp"

#include <filesystem>

void Features::backup_game(const Game& game) {
    std::cout << "creating backup of: " << game.game_name.c_str() << "!\n";
    fs::path game_backup_dir = backup_dir / game.game_name;

    if(!fs::exists(game_backup_dir)) {
        fs::create_directories(game_backup_dir);
    }

    fs::path zip_name = game_backup_dir / construct_backup_name(game);
    Backup::create_backup(zip_name, game);
}

// void Features::restore_game_backup(const Game& game) {
//
//     // std::cout << "restoring backup of: " << game.game_name.c_str() << "!\n";
//     //
//     int backup_count = 0;
//     for(const auto& b : backups) {
//         backup_count += 1;
//         std::cout << backup_count << ". Backup: " << b << "\n";
//     }
//
//     // int backup_selection = get_int(
//     //     "Select a backup (1-" + std::to_string(backup_count) + "): ",
//     //     1, backup_count
//     // );
//     //
//     // const auto& selected_backup = backups[backup_selection - 1];
//
//     // Backup::restore_backup(selected_backup, game);
// }
//
