#include "ubi.hpp"
#include "core/helpers/translations.hpp"

void ubi::find_saves(const fs::path& prefix, std::vector<Game>& out_games, std::string& out_uuid) {
    if(!fs::exists(prefix)) {
        return;
    }

    for(const auto& uuid_entry : fs::directory_iterator(prefix)) {
        fs::path uuid_folder = uuid_entry.path();

        if(out_uuid.empty()) {
            out_uuid = uuid_folder.filename().string();
        }

        for(const auto& game_entry : fs::directory_iterator(uuid_folder)) {
            fs::path game_id_folder = game_entry.path();

            Game game;
            game.type = UBISOFT;
            game.game_id = game_id_folder.filename().string();
            game.game_name = translations::get_game_name_ubi(game.game_id.value()).value_or("Unknown Game");
            game.appid = translations::get_steam_id(game.game_name).value_or("N/A");
            game.save_path = game_id_folder;

            out_games.push_back(game);
        }
    }
}
