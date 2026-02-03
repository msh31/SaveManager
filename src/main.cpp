#include <iostream>

#include "config/config.hpp"
#include "detection/detection.hpp"
#include "helpers/utils.hpp"
#include "command/command.hpp"
#include "ui/menu.hpp"

int main() {
    if(!Config::config_exist()) {
        std::cerr << "Config is missing and could not be generated!\n";
        return 1;
    }

    auto result = Detection::find_saves();
    if(result.games.empty()) {
        std::cerr << "No Ubisoft savegames found, exiting..\n";
        return 1;
    }

    Menu main_menu("Save Manager");
    main_menu.add_item("List games", handle_list);
    main_menu.add_item("Backup", handle_backup);
    main_menu.add_item("Restore", handle_restore);
    main_menu.add_exit_item("Quit");

    while(main_menu.run(result)) {

    }
    return 0;
}
