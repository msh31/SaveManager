#include "unreal.hpp"
#include "core/helpers/translations.hpp"

void unreal::find_saves(const fs::path& prefix, std::vector<Game>& out_games) {
    //in here i would just do similar to the code below which is from RSG,
    //but then go through the generic prefix and look for .sav files since 
    //i am not sure how to resolve the username, likely involves syscalls anyway.

    // if(!fs::exists(prefix)) {
    //     return;
    // }
    //
    // for(const auto& game : fs::directory_iterator(prefix)) {
    //     fs::path game_folder = game.path();
    //     std::string folder_name = game_folder.filename().string();
    //     //
    //
    //
    //     for(const auto& profile : fs::directory_iterator(game_folder)) {
    //         fs::path uuid_folder = profile.path();
    //
    //         Game game;
    //         game.type = UNREAL;
    //         game.game_name = get_game_name_rsg(folder_name).value_or(folder_name);
    //         game.appid = get_steam_id(game.game_name).value_or("N/A");
    //         game.save_path = uuid_folder;
    //
    //         out_games.push_back(game);
    //     }
    // }
}
