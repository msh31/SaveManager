#include "rsg.hpp"
#include "core/helpers/translations/translations.hpp"

void RockstarDetector::find_saves(const fs::path& prefix, std::vector<Game>& out_games) const {
    if(!fs::exists(prefix)) {
        return;
    }

    for(const auto& game : fs::directory_iterator(prefix, std::filesystem::directory_options::skip_permission_denied)) {
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
            game.game_name = translations::get_game_name_rsg(folder_name).value_or(folder_name);
            game.appid = translations::get_steam_id(game.game_name).value_or("N/A");
            game.save_path = uuid_folder;

            out_games.push_back(game);
        }
    }
}
