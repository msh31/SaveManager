#include "unreal.hpp"
#include "core/helpers/translations/translations.hpp"
#include <core/logger/logger.hpp>

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
                //get_logger().debug(path_str);
                directories.insert(file.parent_path());
            }
        }
    }

    for (const auto& entry : directories) {
        auto it = entry.begin();
        Game game;
        game.type = UNREAL;
        game.save_path = entry;
        std::string found_name;
        bool found_in_translations = false;

        std::vector<std::string> path_comps;
        for (const auto& part : entry) {
            path_comps.push_back(part.string());
        }
        auto comp_it = std::find(path_comps.begin(), path_comps.end(), "SaveGames");
        //get_logger().debug(*comp_it);

        while (comp_it != path_comps.begin()) {
            --comp_it;
            bool is_numeric = std::all_of(comp_it->begin(), comp_it->end(), ::isdigit);

            if (auto the_name = translations.find(*comp_it); the_name != translations.end()) {
                if (*comp_it != "Saved" && *comp_it != "Steam" && *comp_it != "Epic" && !is_numeric) {
                    game.game_name = translations::get_steam_name(the_name->second).value_or(the_name->first.data());
                    game.appid = the_name->second; //translations::get_steam_id(game.game_name).value_or("N/A");
                    found_in_translations = true;
                    break;
                }
            }
            else if (*comp_it != "Saved" && *comp_it != "Steam" && *comp_it != "Epic" && !is_numeric) {
                found_name = *comp_it;
                game.game_name = found_name;
                game.appid = "N/A";
                break;
            }
        }

        if (!found_in_translations) {
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
        }

        if (game.game_name.empty() && !found_name.empty()) {
            game.game_name = found_name;
            game.appid = "N/A";
        }
        out_games.push_back(game);
        //get_logger().debug(game.game_name);
    }
}
