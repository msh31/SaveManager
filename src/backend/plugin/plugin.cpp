#include "plugin.hpp"
#include "backend/utils/paths.hpp"

Plugin::Plugin(std::filesystem::path path) {
    lua.open_libraries(sol::lib::base); //allow print, type, tostring etc

    lua.set_function("path_exists", [](const std::string& p) {
            return fs::exists(p);
            });

    lua.set_function("home_dir", []() {
            return paths::home_dir().string();
            });

    lua.script_file(path); //load and run the script
}

//TODO
std::vector<Game> Plugin::find_saves() {
    sol::protected_function fn = lua["find_saves"];
    auto result = fn();
    std::vector<Game> games;

    sol::table table = result;
    for (auto& [key, val] : table) {
        sol::table entry = val.as<sol::table>();
        Game g;
        g.game_name = entry["game_name"].get<std::string>();
        g.appid = entry["appid"].get<std::string>();
        g.save_path = entry["save_path"].get<std::string>();
        games.emplace_back(g);
    }
    return games;
}
