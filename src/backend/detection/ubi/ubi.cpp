#include "ubi.hpp"
#include "backend/utils/translations/translations.hpp"
// #include "backend/logger/logger.hpp"

std::expected<std::vector<Game>, DetectionError> UbisoftDetector::find_saves(const fs::path& prefix) const {
    if(!fs::exists(prefix)) {
        return std::unexpected{DetectionError::PathNotFound};
    }
    std::vector<Game> games;
    // get_logger().debug("scanning path: {}", prefix.string());

    for(const auto& uuid_entry : fs::directory_iterator(prefix, std::filesystem::directory_options::skip_permission_denied)) {
        fs::path uuid_folder = uuid_entry.path();

        if(!fs::is_directory(uuid_folder)) {
            continue;
        }

        for(const auto& game_entry : fs::directory_iterator(uuid_folder)) {
            fs::path game_id_folder = game_entry.path();
            if(!fs::is_directory(game_id_folder)) {
                continue;
            }

            auto name = translations::get_game_name_ubi(game_id_folder.filename().string());
            if(name == std::nullopt) {
                continue;
            }

            Game game;
            game.type = UBISOFT;
            game.game_id = game_id_folder.filename().string();
            game.game_name = name.value();
            game.appid = translations::get_steam_id(game.game_name).value_or("N/A");
            game.save_path = game_id_folder;

            games.push_back(game);
        }
    }
    return games;
}

std::expected<std::vector<Game>, DetectionError>  UbisoftDetector::find_anno_saves(const fs::path& prefix) const {
    if(!fs::exists(prefix)) {
        return std::unexpected{DetectionError::PathNotFound};
    }
    std::vector<Game> games;
    // get_logger().debug("scanning path: {}", prefix.string());

    for(const auto& game : fs::directory_iterator(prefix, std::filesystem::directory_options::skip_permission_denied)) {
        std::string folder_name = game.path().filename().string();

        if (auto it = anno_paths.find(folder_name); it != anno_paths.end()) {
            // get_logger().info("Anno match: " + folder_name + " at " + game.path().string());

            Game anno;
            anno.type = UBISOFT;
            anno.game_name = it->second.game_name;
            anno.appid = translations::get_steam_id(anno.game_name).value_or("N/A");
            fs::path save = game.path() / it->second.save_subpath;
            anno.save_path = fs::exists(save) ? save : game.path(); //a fallback
            // get_logger().info("Anno game: " + anno.game_name + " appid: " + anno.appid);

            games.push_back(anno);
        }
    }
    return games;
}
