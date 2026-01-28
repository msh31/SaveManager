#include <filesystem>
#include <fstream>
#include <iostream>

#include "detection/detection.hpp"
#include "helpers/colors.hpp"
#include "helpers/ascii_art.hpp"
#include "helpers/startup.hpp"
#include "helpers/utils.hpp"


std::string backup_dir = std::string(std::getenv("HOME")) + "/.config/savemanager/backup/";

#include <zip.h>

int main(int argc, char* argv[]) {
    if(!configExists()) {
        std::cerr << "Config is missing and could not be generated!\n";
        return 1;
    }

    if (argc < 2) {
        std::cout << "Usage: savemanager [list|backup|restore]\n";
        return 1;
    }

    std::cout << COLOR_RED << printTitle() << COLOR_RESET << "\n\n";

    std::string command = argv[1];
    auto result = Detection::findSaves();

    if (command == "list" || command == "ls") {
        if(result.games.empty()) {
            std::cerr << "No Ubisoft savegames found!\n";
            return 1;
        }

        std::cout << COLOR_GREEN << "Found profile: " << COLOR_RESET << result.uuid << "\n";

        for(const auto& g : result.games) {
            std::cout << COLOR_RED << "Game Name: " << COLOR_RESET << g.game_name << "\n";
            std::cout << COLOR_BLUE << "Game ID: " << COLOR_RESET << g.game_id;
            std::cout << COLOR_GREEN << " Appid: " << COLOR_RESET << g.appid;
            std::cout << COLOR_YELLOW << " Path: " << COLOR_RESET << g.save_path << "\n\n";
        }
    }
    else if (command == "backup") {
        if(result.games.empty()) {
            std::cerr << "No Ubisoft savegames found, exiting..\n";
            return 1;
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
        std::string zip_name = backup_dir + construct_backup_name(selected_game);
        zip_t* archive = zip_open(zip_name.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &zip_error);

        int file_count = 0;
        for (const auto& entry : fs::recursive_directory_iterator(selected_game.save_path)) {
            if (entry.is_regular_file()) {
                fs::path relative = fs::relative(entry.path(), selected_game.save_path);
                std::cout << "Adding: " << relative << " to the backup for " << selected_game.game_name << "\n";

                zip_source_t* source = zip_source_file(archive, entry.path().c_str(), 0, 0);
                if (!source) {
                    std::cerr << "Failed to create source for: " << entry.path() << "\n";
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
    }
    else if (command == "restore") {
        if(result.games.empty()) {
            std::cerr << "No Ubisoft savegames found, exiting..\n";
            return 1;
        }

        if(fs::is_empty(backup_dir)) {
            std::cerr << "No backups were found, exiting..\n";
            return 1;
        }

        std::vector<fs::path> backups;
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
                if (fileName.find(selected_game.game_id) != std::string::npos)
                {
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
        //TODO

        int zip_error;
        zip_t* archive = zip_open(selected_backup.c_str(), 0, &zip_error);
        int file_count = zip_get_num_entries(archive, 0);

        for (int i = 0; i < file_count; i++) {
            struct zip_stat fileInfo;
            zip_stat_init(&fileInfo); 

            if (zip_stat_index(archive, i, 0, &fileInfo) == 0) {
                std::cout << "File Name: " << fileInfo.name << "\n Saving to: \n";
                const auto& output_path = selected_game.save_path / fileInfo.name;
                std::cout << output_path << "\n";

                zip_file* file = zip_fopen_index(archive, i, 0);
                if (file) {
                    char buffer[1024];

                    ssize_t bytes_read;
                    fs::create_directories(output_path.parent_path());
                    std::ofstream save_file(output_path, std::ios::binary);
                    while ((bytes_read = zip_fread(file, buffer, sizeof(buffer))) > 0) {
                        save_file.write(buffer, bytes_read);
                    }
                    zip_fclose(file);
                }
            }
        }

        zip_close(archive);
        // std::cout << "\nbackup for: " << selected_game.game_name << " has been restored!\n";
    }
    else {
        std::cout << "Unknown command: " << command << "\n";
        return 1;
    }
    return 0;
}
