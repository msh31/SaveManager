#include <iostream>

#include "detection/detection.hpp"
#include "helpers/utils.hpp"
#include "command/command.hpp"

int main() {
    #ifndef _WIN32
    std::cout << COLOR_RED << print_title() << COLOR_RESET << "\n\n";
    #endif

    if(!config_exists()) {
        std::cerr << "Config is missing and could not be generated!\n";
        return 1;
    }


    auto result = Detection::find_saves();
    if(result.games.empty()) {
        std::cerr << "No Ubisoft savegames found, exiting..\n";
        return 1;
    }

    while(true) {
        print_menu();
        int selection = 0;
        std::cin >> selection;

        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input, try again.\n";
            continue;
        }

        switch(selection) {
            case LIST: handle_list(result); break;
            case BACKUP: handle_backup(result); break;
            case RESTORE: handle_restore(result); break;
            case QUIT: return 0;
        }
    }
    return 0;
}
