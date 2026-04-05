#include "unreal.hpp"
#include "backend/utils/translations/translations.hpp"
#include <backend/logger/logger.hpp>

void UnrealDetector::scan_for_saves(const fs::path& path, std::set<fs::path>& directories) const {
    try {
        for (const auto& entry : fs::recursive_directory_iterator(path, fs::directory_options::skip_permission_denied)) {
            if (entry.path().extension() != ".sav") {
                continue;
            }

            std::ifstream save(entry.path(), std::ifstream::binary);
            if (!save.is_open()) {
                continue;
            }

            char buffer[4];
            save.read(buffer, 4);
            if (save.gcount() != 4) {
                continue;
            }

            if (!std::equal(std::begin(buffer), std::end(buffer), std::begin(header))) {
                continue;
            }

            std::string path_str = entry.path().parent_path().string();
            if (path_str.find("Ubisoft") != std::string::npos ||
                path_str.find("Rockstar") != std::string::npos ||
                path_str.find("Application Data BACKUP") != std::string::npos ||
                path_str.find("Settings") != std::string::npos) {
                continue;
            }

            // get_logger().debug("scan_for_saves: " + path.string());
            directories.insert(entry.path().parent_path());
        }
    } catch(std::filesystem::filesystem_error&) {
        return;
    }
}

std::expected<std::vector<Game>, DetectionError> UnrealDetector::find_saves(const fs::path& prefix, UnrealDetector::ScanMode scan_mode) const {
    std::set<fs::path> directories;
    std::vector<Game> games;

    if(!fs::exists(prefix)) {
        return std::unexpected{DetectionError::PathNotFound};
    }

    switch (scan_mode) {
        case UnrealDetector::ScanMode::Recursive:
            scan_for_saves(prefix, directories);
            break;
        case UnrealDetector::ScanMode::Native:
            for(const auto& folder : fs::directory_iterator(prefix, std::filesystem::directory_options::skip_permission_denied)) {
                if (!fs::is_directory(folder)) {
                    continue;
                }

                fs::path save_games = folder.path() / "Saved" / "SaveGames";
                fs::path save_games_alt = folder.path() / "SaveGames";

                fs::path target;
                if (fs::exists(save_games)) {
                    target = save_games;
                }
                else if (fs::exists(save_games_alt)) { 
                    target = save_games_alt;
                }
                else {
                    try {
                        for (const auto& subfolder : fs::directory_iterator(folder, fs::directory_options::skip_permission_denied)) {
                            if (!fs::is_directory(subfolder)) {
                                continue;
                            }
                            fs::path sub_save_games = subfolder.path() / "Saved" / "SaveGames";
                            fs::path sub_save_games_alt = subfolder.path() / "SaveGames";
                            fs::path target_two;

                            if (fs::exists(sub_save_games)) {
                                target_two = sub_save_games;
                            }
                            else if (fs::exists(sub_save_games_alt)) {
                                target_two = sub_save_games_alt;
                            } else {
                                continue;
                            }
                            // get_logger().debug("target_two: " + target_two.string());
                            scan_for_saves(target_two, directories);
                        }
                    } catch (const fs::filesystem_error&) {
                        continue;
                    }
                    continue;
                }

                try {
                    scan_for_saves(target, directories);
                } catch (const fs::filesystem_error&) {
                    continue;
                }
            }
            break;
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
        games.push_back(game);
    }

    return games;
}
