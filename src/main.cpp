#include <iostream>

#include "detection/detection.hpp"

// Detection detect;

int main(int argc, char* argv[]) {
    auto games = Detection::findSaves();

    if(games.empty()) {
        std::cerr << "No Ubisoft savegames found!\n";
        return 1;
    }

    for(const auto& g : games) {
        std::cout << "Game ID: " << g.game_id 
            << " (appid: " << g.appid << ")"
            << " (path: " << g.save_path << ")\n";
    }

    // if (argc < 2) {
    //     std::cout << "Usage: savemanager\n";
    //     return 1;
    // }
    return 0;
}
