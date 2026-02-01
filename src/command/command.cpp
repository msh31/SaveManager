#include "command.hpp"

void print_menu() {
    std::cout << "1. List saves\n";
    std::cout << "2. Backup\n";
    std::cout << "3. Restore\n";
    std::cout << "4. Quit\n";
    std::cout << "> ";
}

void handle_list(const Detection::DetectionResult& result) {
    if(result.games.empty()) {
        std::cerr << "No Ubisoft savegames found, exiting..\n";
        return;
    }

    std::cout << COLOR_GREEN << "Found profile: " << COLOR_RESET << result.uuid << "\n";

    for(const auto& g : result.games) {
        std::cout << COLOR_RED << "Game Name: " << COLOR_RESET << g.game_name << "\n";
        std::cout << COLOR_BLUE << "Game ID: " << COLOR_RESET << g.game_id;
        std::cout << COLOR_GREEN << " Appid: " << COLOR_RESET << g.appid;
        std::cout << COLOR_YELLOW << " Path: " << COLOR_RESET << g.save_path << "\n\n";
    }
    wait_for_key();
}

void handle_backup(const Detection::DetectionResult& result) {

    if(result.games.empty()) {
        std::cerr << "No Ubisoft savegames found, exiting..\n";
        return;
    }

    int count = 0;
    for(const auto& g : result.games) {
        count += 1;
        std::cout << count << ". " << COLOR_RED << "Game Name: " << COLOR_RESET << g.game_name << "\n";
    }

    int selection = 0;
    std::cout << "Select game to backup (1-" << count << "): ";
    std::cin >> selection;

    while (selection < 1 || selection > count) {
        std::cerr << "Invalid selection\n > ";
        std::cin >> selection;
    }

    const auto& selected_game = result.games[selection - 1];

    std::cout << "Backing up: " << COLOR_BLUE << selected_game.game_name << COLOR_RESET << "\n\n";
    // std::cout << "Save path: " << selected_game.save_path << "\n";
    //

    int zip_error;
    fs::path zip_name = backup_dir / construct_backup_name(selected_game);
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
        std::cerr << "No Ubisoft savegames found, exiting..\n";
        return;
    }

    if(fs::is_empty(backup_dir)) {
        std::cerr << "No backups were found, exiting..\n";
        return;
    }

    std::vector<fs::path> backups;
    std::vector<std::string> failed_files;

    int count = 0;
    for(const auto& g : result.games) {
        count += 1;
        std::cout << count << ". " << COLOR_RED << "Game Name: " << COLOR_RESET << g.game_name << "\n";
    }

    int selection = 0;
    std::cout << "Select a game (1-" << count << "): ";
    std::cin >> selection;

    while (selection < 1 || selection > count) {
        std::cerr << "Invalid selection\n > ";
        std::cin >> selection;
    }

    const auto& selected_game = result.games[selection - 1];

    for (const auto& entry : fs::recursive_directory_iterator(backup_dir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".zip") {
            const auto& full_path = entry.path();
            // std::cout << full_path << "\n";

            const std::string fileName = entry.path().filename().string();
            if (fileName.find(selected_game.game_id) != std::string::npos) {
                backups.emplace_back(full_path);
            }
        }
    }

    int backup_count = 0;
    for(const auto& b : backups) {
        backup_count += 1;
        std::cout << backup_count << ". " << COLOR_RED << "Backup: " << COLOR_RESET << b << "\n";
    }

    int backup_selection = 0;
    std::cout << "Select a backup (1-" << backup_count << "): ";
    std::cin >> backup_selection;

    while (backup_selection < 1 || backup_selection > backup_count) {
        std::cerr << "Invalid selection\n > ";
        std::cin >> backup_selection;
    }

    const auto& selected_backup = backups[backup_selection - 1];

    std::cout << "Restoring backup for: " << COLOR_BLUE << selected_game.game_name << COLOR_RESET << "\n\n";

    int zip_error;
    std::string selected_backup_utf8 = selected_backup.u8string();
    zip_t* archive = zip_open(selected_backup_utf8.c_str(), 0, &zip_error);

    if(!archive) {
        std::cout << COLOR_RED << "Could not open backup for restoration process!\n" << COLOR_RESET;
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
        }
    } else {
        std::cout << "\nbackup for: " << selected_game.game_name << " has been restored!\n";
    }
    wait_for_key();
}
