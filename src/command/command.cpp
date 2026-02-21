#include "command.hpp"
#include "../ui/input_validator.hpp"
#include "../core/backup/backup.hpp"

#include <filesystem>

void handle_list(const Detection::DetectionResult& result) {
    if(result.games.empty()) {
        std::cerr << "No Ubisoft savegames found, exiting..\n";
        wait_for_key();
        return;
    }

    std::cout << COLOR_GREEN << "Found profile: " << COLOR_RESET << result.uuid.value() << "\n";

    for(const auto& g : result.games) {
        std::cout << COLOR_RED << "Game Name: " << COLOR_RESET << g.game_name << "\n";
        std::cout << COLOR_BLUE << "Game ID: " << COLOR_RESET << g.game_id.value_or("N/A");
        std::cout << COLOR_GREEN << " Appid: " << COLOR_RESET << g.appid;
        std::cout << COLOR_YELLOW << " Path: " << COLOR_RESET << g.save_path << "\n\n";
    }
    wait_for_key();
}

void handle_backup(const Detection::DetectionResult& result) {
    const Game* selected_game = Detection::get_selected_game(result);

    if(!selected_game) {
        return;
    }

    std::cout << "Backing up: " << COLOR_BLUE << selected_game->game_name << COLOR_RESET << "\n\n";

    std::string custom_title;
    std::cout << COLOR_BLUE << "Enter a custom backup name (Ex: hard_100_percent): " << COLOR_RESET;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::getline(std::cin, custom_title);

    int zip_error;

    fs::path game_backup_dir = selected_game->game_name;
    if(!fs::exists(backup_dir / game_backup_dir)) {
        fs::create_directories(backup_dir / game_backup_dir);
    }

    fs::path zip_name;
    if(custom_title.empty()) {
        zip_name = backup_dir / game_backup_dir / construct_backup_name(*selected_game);
    } else {
        zip_name = backup_dir / game_backup_dir / construct_backup_name(*selected_game, custom_title);
    }

    Backup::create_backup(zip_name, *selected_game);
    wait_for_key();
}

void handle_restore(const Detection::DetectionResult& result) {
    if(fs::is_empty(backup_dir)) {
        std::cerr << "No backups were found!\n";
        wait_for_key();
        return;
    }

    const Game* selected_game = Detection::get_selected_game(result);

    if(!selected_game) {
        return;
    }

    std::vector<fs::path> backups;

    fs::path game_backup_dir = selected_game->game_name;
    if(!fs::exists(backup_dir / game_backup_dir)) {
        std::cerr << COLOR_RED "No backups found for: " << COLOR_RESET << selected_game->game_name << "!\n";
        wait_for_key();
        return;
    }

    for (const auto& entry : fs::recursive_directory_iterator(backup_dir / game_backup_dir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".zip") {
            const auto& full_path = entry.path();
            backups.emplace_back(full_path);
        }
    }

    int backup_count = 0;
    for(const auto& b : backups) {
        backup_count += 1;
        std::cout << backup_count << ". " << COLOR_RED << "Backup: " << COLOR_RESET << b << "\n";
    }

    if(backup_count <= 0) {
        std::cerr << COLOR_RED << "No backups found!\n" << COLOR_RESET;
        wait_for_key();
        return;
    }

    int backup_selection = get_int(
        "Select a backup (1-" + std::to_string(backup_count) + "): ",
        1, backup_count
    );

    const auto& selected_backup = backups[backup_selection - 1];

    std::cout << "Restoring backup for: " << COLOR_BLUE << selected_game->game_name << COLOR_RESET << "\n\n";

    Backup::restore_backup(selected_backup, *selected_game);
    wait_for_key();
}
