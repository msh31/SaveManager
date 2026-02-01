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
        return "macOS";
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
    std::vector<Game> games;
    std::string found_uuid;

#ifdef __linux__
    auto libraries = Detection::get_library_folders();
    if(libraries.empty()) {
        std::cerr << "No steam libraries found!\n";
        return {};
    }

    for (auto library : libraries) {
        #ifdef __linux__
        fs::path compatdata_path = library / "steamapps/compatdata";

        if(!fs::exists(compatdata_path)) {
            continue;
        }
        
        for(const auto& entry : fs::directory_iterator(compatdata_path)) {
            fs::path appid_folder = entry.path();
            fs::path ubi_save_path = appid_folder / "pfx/drive_c/Program Files (x86)/Ubisoft/Ubisoft Game Launcher/savegames";

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
        #endif // __linux__

        #ifdef _WIN32
        fs::path ubi_save_path = "C:/Program Files (x86)/Ubisoft/Ubisoft Game Launcher/savegames";

        if(fs::exists(ubi_save_path)) {
            std::cout << "Windows support is a work in progress!\n";

            for(const auto& uuid_entry : fs::directory_iterator(ubi_save_path)) {
                fs::path uuid_folder = uuid_entry.path();

                if(found_uuid.empty()) {
                    found_uuid = uuid_folder.filename().string();
                }

                for(const auto& game_entry : fs::directory_iterator(uuid_folder)) {
                    fs::path game_id_folder = game_entry.path();

                    UbiGame game;
                    game.appid = "N/A"; 
                    game.game_id = game_id_folder.filename().string();
                    game.save_path = game_id_folder;
                    game.game_name = getGameName(game.game_id).value_or("Unknown Game");

                    games.push_back(game);
                }
            }
        }
        #endif // _WIN32
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

                UbiGame game;
                game.appid = "N/A";
                game.game_id = game_id_folder.filename().string();
                game.save_path = game_id_folder;
                game.game_name = getGameName(game.game_id).value_or("Unknown Game");

                games.push_back(game);
            }
        }
    }
#endif // _WIN32

    return {found_uuid, games};
}
