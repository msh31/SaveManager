#include "games.hpp"

bool game_detection::loadGameDatabase(const std::string &jsonPath) {
    std::ifstream file(jsonPath);

    if(!file.is_open()) {
        log.error("Could not load the game database!");
        return false;
    }

    try {
        gameDatabase = json::parse(file);
        return true;
    } catch (const json::exception& e) {
        log.error("Failed to parse game database: " + std::string(e.what()));
    }

    return false;
}
