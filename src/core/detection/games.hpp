#pragma once
#include "../savemanager.hpp"

namespace game_detection {
static json gameDatabase; 
static logger log;

bool loadGameDatabase(const std::string& jsonPath);

std::vector<GameInfo> scanSteamLibrary(const std::string& libraryPath, 
                                       const std::string& profileID);

int matchDocumentFolder(const std::string& folderName);
}
