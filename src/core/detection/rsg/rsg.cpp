#include "rsg.hpp"
#include "core/helpers/translations.hpp"

void rsg::find_saves(const fs::path& prefix, std::vector<Game>& out_games) {
    if(!fs::exists(prefix)) {
        return;
    }

    for(const auto& game : fs::directory_iterator(prefix)) {
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
            game.game_name = game_folder.filename().string();
            game.appid = get_steam_id(game.game_name).value_or("N/A");
            game.save_path = uuid_folder;

            out_games.push_back(game);
        }
    }
}
