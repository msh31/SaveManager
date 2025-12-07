#include "games.hpp"
#include "savemanager.hpp"

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

std::vector<GameInfo> game_detection::scanSteamLibrary(const std::string &libraryPath, const std::string &profileID) {
    std::vector<GameInfo> foundGames;
    fs::path compatdataPath = fs::path(libraryPath) / "steamapps" / "compatdata";

    if (!fs::exists(compatdataPath)) {
        log.error("Compatdata path does not exist: " + compatdataPath.string());
        return foundGames;
    }

    if (!fs::is_directory(compatdataPath)) {
        log.error("Compatdata path is not a directory!");
        return foundGames;
    }

    for (const auto& appidEntry : fs::directory_iterator(compatdataPath)) {
        if (!appidEntry.is_directory()) {
            continue;
        }

        fs::path documentsPath = appidEntry.path() / "pfx/drive_c/users/steamuser/Documents";

        if (!fs::exists(documentsPath)) {
            continue;
        }

        for (const auto& docFolder : fs::directory_iterator(documentsPath)) {
            if (!docFolder.is_directory()) {
                continue;
            }

            std::string folderName = docFolder.path().filename().string();
            int gameID = matchDocumentFolder(folderName);

            if (gameID != -1) {  
                GameInfo game; 

                std::string saveSubpath;
                for (const auto& g : gameDatabase["ubisoft_games"]) {
                    if (g["appid"].get<int>() == gameID) {
                        game.gameName = g["name"].get<std::string>();
                        saveSubpath = g["save_subpath"].get<std::string>();
                        break;
                    }
                }

                fs::path savePath = docFolder.path() / saveSubpath / profileID;

                if (!fs::exists(savePath)) {
                    continue;  
                }
                game.savePath = savePath.string();

                int count = 0;
                for (const auto& saveFile : fs::directory_iterator(savePath)) {
                    if (saveFile.is_regular_file()) {
                        count++;
                    }
                }

                game.saveCount = count;
                game.lastModified = fs::last_write_time(savePath);
                game.gameID = gameID;
                game.documentFolder = folderName;
                game.platform = PlatformTypes::Ubisoft;

                foundGames.push_back(game);
            }
        }
    }

    return foundGames;
}

int game_detection::matchDocumentFolder(const std::string& folderName) {
    if (gameDatabase.empty()) {
        log.error("Game database not loaded!");
        return -1;
    }

    for (const auto& game : gameDatabase["ubisoft_games"]) {
        if(game["document_folder"] == folderName) {
            return game["appid"].get<int>();
        }
    }

    return -1;
}
