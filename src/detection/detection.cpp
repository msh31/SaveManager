#include <filesystem>

#include "detection.hpp"
#include "../helpers/ubi_name_translations.hpp"

//linux only for now
std::optional<fs::path> Detection::getSteamLocation() {
    std::string home = std::getenv("HOME");
    std::string path1 = home + "/.steam/steam/steamapps/libraryfolders.vdf";
    std::string path2 = home + "/.local/share/Steam/steamapps/libraryfolders.vdf";

    if(fs::exists(path1)) {
        return path1;
    }

    if(fs::exists(path2)) {
        return path2;
    }

    return std::nullopt;
}

//PUBLIC

std::vector<fs::path> Detection::getLibraryFolders() {
    auto vdf_file = getSteamLocation();
    std::vector<fs::path> libraries;

    if(!vdf_file) {
        return {};
    }

    std::ifstream file = vdf_file.value();
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

Detection::DetectionResult Detection::findSaves() {
    auto libraries = Detection::getLibraryFolders();
    std::vector<UbiGame> games;
    std::string found_uuid;

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

            if(fs::exists(ubi_save_path)) {
                for(const auto& uuid_entry : fs::directory_iterator(ubi_save_path)) {
                    fs::path uuid_folder = uuid_entry.path();

                    if(found_uuid.empty()) {
                        found_uuid = uuid_folder.filename().string();
                    }

                    for(const auto& game_entry : fs::directory_iterator(uuid_folder)) {
                        fs::path game_id_folder = game_entry.path();

                        UbiGame game;
                        game.appid = appid_folder.filename().string();
                        game.game_id = game_id_folder.filename().string();
                        game.save_path = game_id_folder;
                        game.game_name = getGameName(game.game_id).value_or("Unknown Game");

                        games.push_back(game);
                    }
                }
            }
        }
    }

    return {found_uuid, games};
}
