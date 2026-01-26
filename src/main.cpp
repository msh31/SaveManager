#include <iostream>

#include "detection/detection.hpp"
#include "helpers/colors.hpp"
#include "helpers/ascii_art.hpp"
#include "helpers/startup.hpp"
#include "helpers/utils.hpp"

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
            std::cerr << "No Ubisoft savegames found!\n";
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

        if (selection < 1 || selection > count) {
            std::cerr << "Invalid selection\n";
            return 1;
        }

        const auto& selected_game = result.games[selection - 1];

        std::cout << "Backing up: " << COLOR_BLUE << selected_game.game_name << COLOR_RESET << "\n\n";
        // std::cout << "Save path: " << selected_game.save_path << "\n";
        //

        int zip_error;
        std::string backup_dir = std::string(std::getenv("HOME")) + "/.config/savemanager/backup/";
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
        std::cout << command << " is a work in progress!" << "\n";
    }
    else {
        std::cout << "Unknown command: " << command << "\n";
        return 1;
    }
    return 0;
}
