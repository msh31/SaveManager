#include "command.hpp"
#include "../ui/input_validator.hpp"
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
    if(result.games.empty()) {
        std::cerr << "No Ubisoft savegames found!\n";
        wait_for_key();
        return;
    }

    int count = 0;
    for(const auto& g : result.games) {
        count += 1;
        std::cout << count << ". " << COLOR_RED << "Game Name: " << COLOR_RESET << g.game_name << "\n";
    }

    if(count <= 0) {
        std::cerr << "No valid games found!\n";
        wait_for_key();
        return;
    }

    int selection = get_int(
        "Select game to backup (1-" + std::to_string(count) + "): ",
        1, count
    );

    const auto& selected_game = result.games[selection - 1];

    std::cout << "Backing up: " << COLOR_BLUE << selected_game.game_name << COLOR_RESET << "\n\n";
    // std::cout << "Save path: " << selected_game.save_path << "\n";
    //

    std::string custom_title;
    std::cout << COLOR_BLUE << "Enter a custom backup name (Ex: hard_100_percent): " << COLOR_RESET;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::getline(std::cin, custom_title);

    int zip_error;

    fs::path game_backup_dir = selected_game.game_name;
    if(!fs::exists(backup_dir / game_backup_dir)) {
        fs::create_directories(backup_dir / game_backup_dir);
    }

    fs::path zip_name;
    if(custom_title.empty()) {
        zip_name = backup_dir / game_backup_dir / construct_backup_name(selected_game);
    } else {
        zip_name = backup_dir / game_backup_dir / construct_backup_name(selected_game, custom_title);
    }

    std::string zip_name_utf8 = zip_name.u8string();
    zip_t* archive = zip_open(zip_name_utf8.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &zip_error);

    if(!archive) {
        std::cout << COLOR_RED << "Could not create backup!\n" << COLOR_RESET;
        return;
    }

    int file_count = 0;
    for (const auto& entry : fs::recursive_directory_iterator(selected_game.save_path)) {
        if (entry.is_regular_file()) {
            std::string entry_utf8 = entry.path().u8string();
            fs::path relative = fs::relative(entry_utf8.c_str(), selected_game.save_path);
            std::cout << "Adding: " << relative << " to the backup for " << selected_game.game_name << "\n";

            zip_source_t* source = zip_source_file(archive, entry_utf8.c_str(), 0, 0);
            if (!source) {
                std::cerr << "Failed to create source for: " << entry_utf8.c_str() << "\n";
                continue;
            }

            if (zip_file_add(archive, relative.string().c_str(), source, ZIP_FL_OVERWRITE) < 0) {
                std::cerr << "Failed to add file: " << zip_strerror(archive) << "\n";
            }
            file_count++;
        }
    }
    std::cout << "Added " << file_count << " files\n";
    zip_close(archive);
    std::cout << "\nbackup has been created!\n";
    wait_for_key();
}

void handle_restore(const Detection::DetectionResult& result) {
    if(result.games.empty()) {
        std::cerr << "No Ubisoft savegames found!\n";
        wait_for_key();
        return;
    }

    if(fs::is_empty(backup_dir)) {
        std::cerr << "No backups were found!\n";
        wait_for_key();
        return;
    }

    std::vector<fs::path> backups;
    std::vector<std::string> failed_files;

    int count = 0;
    for(const auto& g : result.games) {
        count += 1;
        std::cout << count << ". " << COLOR_RED << "Game Name: " << COLOR_RESET << g.game_name << "\n";
    }

    int selection = get_int(
        "Select a game (1-" + std::to_string(count) + "): ",
        1, count
    );

    const auto& selected_game = result.games[selection - 1];

    fs::path game_backup_dir = selected_game.game_name;
    if(!fs::exists(backup_dir / game_backup_dir)) {
        std::cerr << COLOR_RED "No backups found for: " << COLOR_RESET << selected_game.game_name << "!\n";
        return;
        wait_for_key();
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

    std::cout << "Restoring backup for: " << COLOR_BLUE << selected_game.game_name << COLOR_RESET << "\n\n";

    int zip_error;
    std::string selected_backup_utf8 = selected_backup.u8string();
    zip_t* archive = zip_open(selected_backup_utf8.c_str(), 0, &zip_error);

    if(!archive) {
        std::cout << COLOR_RED << "Could not open backup for restoration process!\n" << COLOR_RESET;
        wait_for_key();
        return;
    }

    int file_count = zip_get_num_entries(archive, 0);
    for (int i = 0; i < file_count; i++) {
        struct zip_stat fileInfo;
        zip_stat_init(&fileInfo); 

        if (zip_stat_index(archive, i, 0, &fileInfo) == 0) {
            std::cout << "File Name: " << fileInfo.name << "\n Saving to: \n";
            const auto& output_path = selected_game.save_path / fileInfo.name;
            std::cout << output_path << "\n";

            zip_file* file = zip_fopen_index(archive, i, 0);

            if (!file) {
                failed_files.push_back(fileInfo.name);
                continue;
            }
            char buffer[1024];

            zip_int64_t bytes_read;
            fs::create_directories(output_path.parent_path());
            std::ofstream save_file(output_path, std::ios::binary);

            while ((bytes_read = zip_fread(file, buffer, sizeof(buffer))) > 0) {
                save_file.write(buffer, bytes_read);
            }
            zip_fclose(file);
        }
    }

    zip_close(archive);
    if (!failed_files.empty()) {
        std::cerr << COLOR_RED << "Failed to restore:\n" << COLOR_RESET;
        for (const auto& f : failed_files) {
            std::cerr << "  - " << f << "\n";
            wait_for_key();
        }
    } else {
        std::cout << "\nbackup for: " << selected_game.game_name << " has been restored!\n";
    }
    wait_for_key();
}
