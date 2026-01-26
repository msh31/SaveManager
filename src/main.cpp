#include <iostream>

#include "detection/detection.hpp"
#include "helpers/colors.hpp"
#include "helpers/ascii_art.hpp"

// Detection detect;

int main(int argc, char* argv[]) {
    std::cout << COLOR_RED << printTitle() << COLOR_RESET << "\n\n";
    auto result = Detection::findSaves();

    if(result.games.empty()) {
        std::cerr << "No Ubisoft savegames found!\n";
        return 1;
    }

    std::cout << COLOR_GREEN << "Found profile: " << COLOR_RESET << result.uuid << "\n";

    for(const auto& g : result.games) {
        std::cout << COLOR_BLUE << "Game ID: " << COLOR_RESET << g.game_id;
        std::cout << COLOR_GREEN << " Appid: " << COLOR_RESET << g.appid;
        std::cout << COLOR_YELLOW << " Path: " << COLOR_RESET << g.save_path << "\n\n";
    }

    // if (argc < 2) {
    //     std::cout << "Usage: savemanager\n";
    //     return 1;
    // }
    return 0;
}
