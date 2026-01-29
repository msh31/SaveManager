#include <iostream>

#include "detection/detection.hpp"
#include "helpers/colors.hpp"
#include "helpers/ascii_art.hpp"
#include "helpers/startup.hpp"
#include "helpers/utils.hpp"
#include "command/command.hpp"

int main() {
    auto result = Detection::findSaves();
    std::cout << COLOR_RED << print_title() << COLOR_RESET << "\n\n";

    if(!config_exists()) {
        std::cerr << "Config is missing and could not be generated!\n";
        return 1;
    }

    if(result.games.empty()) {
        std::cerr << "No Ubisoft savegames found, exiting..\n";
        return 1;
    }

    while(true) {
        print_menu();
        int selection = 0;
        std::cin >> selection;

        switch(selection) {
            case LIST: handle_list(result); break;
            case BACKUP: handle_backup(result); break;
            case RESTORE: handle_restore(result); break;
            case QUIT: return 0;
        }
    }
    return 0;
}
