#include "unreal.hpp"

void UnrealDetector::find_saves(const fs::path& prefix, std::vector<Game>& out_games) const {
    if(!fs::exists(prefix)) {
        return;
    }
    std::set<fs::path> directories;
    fs::path file;
    char header[4] = {'G','V','A','S'};

    for(const auto& folder : fs::recursive_directory_iterator(prefix, std::filesystem::directory_options::skip_permission_denied)) { 
        file = folder.path();

        if(file.extension() == ".sav") {
            std::ifstream save(file, std::ifstream::binary);

            if(!save.is_open()) {
                continue;
            } else {
                char buffer[4];
                save.read(buffer, 4);
                if(save.gcount() != 4) {
                    continue;
                }

                if(!std::equal(std::begin(buffer), std::end(buffer), std::begin(header))) {
                    continue;
                }

                std::string path_str = file.parent_path().string();
                if (path_str.find("Ubisoft") != std::string::npos || 
                    path_str.find("Rockstar") != std::string::npos ||
                    path_str.find("Application Data BACKUP") != std::string::npos ||
                    path_str.find("Settings") != std::string::npos) { //might cause issues but works fine for now
                    continue;
                }
                directories.insert(file.parent_path());
            }
        }
    }

    for (const auto& entry : directories) {
        auto it = entry.begin();
        Game game;
        game.type = UNREAL;
        game.save_path = entry;

        for (; it != entry.end(); ++it) {
            if (*it == "compatdata") {
                ++it; // next component is the appid
                std::string appid = it->string();

                game.game_name = translations::get_steam_name(appid).value_or("N/A");
                game.appid = appid;

                break;
            } else if(*it == "default") {
                ++it;
                std::string name = it->string();
                game.game_name = name;
                game.appid = "N/A";
                break;
            }
        }
        out_games.push_back(game);
    }
}
