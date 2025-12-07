#include <iostream>

#include "../core/savemanager.hpp"

int main() {
    if (!game_detection::loadGameDatabase("./data/games.json")) {
        return 1;
    }
    
    auto games = game_detection::scanSteamLibrary("/mnt/games/SteamLibrary", "185654f7-5a7b-44de-baa7-76c4f26fbda4");

    std::cout << "Found " << games.size() << " game(s)\n\n";

    for (const auto& game : games) {
        std::cout << "Game: " << game.gameName << "\n";
        std::cout << "  ID: " << game.gameID << "\n";
        std::cout << "  Saves: " << game.saveCount << " files\n";
        std::cout << "  Location: " << game.savePath << "\n";
        std::cout << "\n";
    }
    return 0;
}
