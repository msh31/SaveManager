#include <filesystem>

#include "detection.hpp"
#include "../helpers/ubi_name_translations.hpp"

std::vector<std::string> Detection::get_platform_steam_paths() {
    #ifdef __linux__
        return {
            std::string(std::getenv("HOME")) + "/.steam/steam/steamapps/libraryfolders.vdf",
            std::string(std::getenv("HOME")) + "/.local/share/Steam/steamapps/libraryfolders.vdf"
        };
    #endif
    
    #ifdef _WIN32
        return {
            "C:\\Program Files (x86)\\Steam\\steamapps\\libraryfolders.vdf",
        };
    #endif
    
    #ifdef __APPLE__
        return {
            "macOS"
        };
    #endif
}

std::optional<fs::path> Detection::get_steam_location() {
    for (auto entry : get_platform_steam_paths()) {
        if(fs::exists(entry)) {
            return entry;
        }
    }

    return std::nullopt;
}

Detection::DetectionResult Detection::find_ubi_saves() {
    std::vector<Game> games;
    std::string found_uuid;

#ifdef __linux__
    auto libraries = Detection::get_library_folders();
    if(libraries.empty()) {
        std::cerr << "No steam libraries found!\n";
        return {};
    }

    for (auto library : libraries) {
        fs::path compatdata_path = library / "steamapps/compatdata";

        if(!fs::exists(compatdata_path)) {
            continue;
        }
        
        for(const auto& entry : fs::directory_iterator(compatdata_path)) {
            fs::path appid_folder = entry.path();
            fs::path ubi_save_path = appid_folder / "pfx/drive_c/Program Files (x86)/Ubisoft/Ubisoft Game Launcher/savegames";
            fs::path ac_save_path = appid_folder / "pfx/drive_c/users/steamuser/AppData/Roaming/Ubisoft/Assassin's Creed/Saved Games/";

            if(fs::exists(ac_save_path)) {
                Game game;
                game.type = UBISOFT;
                game.appid = "N/A"; //could assign the steamID, but both uplay and steam store that path.. kinda pointless
                game.game_id = "82";
                game.save_path = ac_save_path;
                game.game_name = "Assassin's Creed";
                games.push_back(game);
            }

            if(fs::exists(ubi_save_path)) {
                for(const auto& uuid_entry : fs::directory_iterator(ubi_save_path)) {
                    fs::path uuid_folder = uuid_entry.path();

                    if(found_uuid.empty()) {
                        found_uuid = uuid_folder.filename().string();
                    }

                    for(const auto& game_entry : fs::directory_iterator(uuid_folder)) {
                        fs::path game_id_folder = game_entry.path();

                        Game game;
                        game.type = UBISOFT;
                        game.appid = appid_folder.filename().string();
                        game.game_id = game_id_folder.filename().string();
                        game.save_path = game_id_folder;
                        game.game_name = getGameName(game.game_id.value()).value_or("Unknown Game");

                        games.push_back(game);
                    }
                }
            }
        }
    }
#endif // __linux__

#ifdef _WIN32
    fs::path ubi_save_path = "C:\\Program Files (x86)\\Ubisoft\\Ubisoft Game Launcher\\savegames";

    if (fs::exists(ubi_save_path)) {
        for (const auto& uuid_entry : fs::directory_iterator(ubi_save_path)) {
            fs::path uuid_folder = uuid_entry.path();

            if (found_uuid.empty()) {
                found_uuid = uuid_folder.filename().string();
            }

            for (const auto& game_entry : fs::directory_iterator(uuid_folder)) {
                fs::path game_id_folder = game_entry.path();

                Game game;
                game.appid = "N/A";
                game.game_id = game_id_folder.filename().string();
                game.save_path = game_id_folder;
                game.game_name = getGameName(game.game_id.value()).value_or("Unknown Game");

                games.push_back(game);
            }
        }
    }
#endif // _WIN32
    return {found_uuid, games};
}

Detection::DetectionResult Detection::find_rsg_saves() {
    std::vector<Game> games;
    std::string found_uuid;

#ifdef __linux__
    auto libraries = Detection::get_library_folders();
    if(libraries.empty()) {
        std::cerr << "No steam libraries found!\n";
        return {};
    }

    for (auto library : libraries) {
        fs::path compatdata_path = library / "steamapps/compatdata";

        if(!fs::exists(compatdata_path)) {
            continue;
        }
        
        for(const auto& entry : fs::directory_iterator(compatdata_path)) {
            fs::path appid_folder = entry.path();
            fs::path rsg_root = appid_folder / "pfx/drive_c/users/steamuser/Documents/Rockstar Games/";

            if(!fs::exists(rsg_root)) {
                continue;
            }

            for(const auto& game : fs::directory_iterator(rsg_root)) {
                fs::path game_folder = game.path();
                std::string folder_name = game_folder.filename().string();
                fs::path profiles_folder = game_folder / "Profiles";

                if(folder_name == "Launcher" || folder_name == "Social Club") {
                    continue;
                }

                if(!fs::exists(profiles_folder)) {
                    continue;
                }

                for(const auto& profile : fs::directory_iterator(profiles_folder)) {
                    fs::path uuid_folder = profile.path();

                    Game game;
                    game.type = ROCKSTAR;
                    game.appid = appid_folder.filename().string();
                    game.save_path = uuid_folder;
                    game.game_name = game_folder.filename().string();

                    games.push_back(game);
                }
            }
        }
    }
#endif // __linux__

#ifdef _WIN32
    //TODO
    Game game;
    game.type = ROCKSTAR;
    game.appid = "69";
    game.save_path = "test";
    game.game_name = "Red Dead Revolver"; 

    games.push_back(game);
#endif // _WIN32
    return {found_uuid, games};
}
//PUBLIC

std::vector<fs::path> Detection::get_library_folders() {
    auto vdf_file = get_steam_location();
    std::vector<fs::path> libraries;

    if(!vdf_file) {
        return {};
    }

    std::ifstream file(vdf_file.value().string());
    std::string line;

    if(!file.is_open()) {
        return {};
    }

    while (std::getline(file, line)) {
        if(line.find("\"path\"") != std::string::npos) {
            size_t first_quote = line.find('"');
            size_t second_quote = line.find('"', first_quote + 1);
            size_t third_quote = line.find('"', second_quote + 1);
            size_t fourth_quote = line.find('"', third_quote + 1);

            if(fourth_quote == std::string::npos) {
                continue;
            }

            std::string path_value = line.substr(third_quote + 1, fourth_quote - third_quote - 1);
            libraries.push_back(path_value);
        }
    }

    file.close();
    return libraries;
}


Detection::DetectionResult Detection::find_saves() {
    DetectionResult result;
    auto ubi_result = Detection::find_ubi_saves();
    auto rsg_result = Detection::find_rsg_saves();

    if(ubi_result.games.empty()) {
        std::cerr << "No Ubisoft savegames found!\n";
    }

    if(rsg_result.games.empty()) {
        std::cerr << "No Rockstar Games savegames found!\n";
    }

    result.uuid = ubi_result.uuid; //might not be strictly needed!
    result.games.insert(result.games.end(), ubi_result.games.begin(), ubi_result.games.end());
    result.games.insert(result.games.end(), rsg_result.games.begin(), rsg_result.games.end());
    return result;
}

const Game* Detection::get_selected_game(const DetectionResult &result) {
    if(result.games.empty()) {
        std::cerr << "No Savegames found!\n";
        wait_for_key();
        return nullptr;
    }

    int count = 0;
    for (const auto& g : result.games) {
        count += 1;
        std::cout << count << ". " << COLOR_RED << "Game Name: " << COLOR_RESET << g.game_name << "\n";
    }

    int selection = get_int(
        "Select a game (1-" + std::to_string(count) + "): ",
        1, count
    );

    return &result.games[selection - 1];
}

