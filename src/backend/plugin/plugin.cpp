#include "plugin.hpp"
#include "backend/logger/logger.hpp"
#include "backend/utils/paths.hpp"

Plugin::Plugin(std::filesystem::path path) {
    lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::table); //allow print, type, tostring etc

    lua.set_function("path_exists", [](const std::string& p) {
            return fs::exists(p);
            });

    lua.set_function("home_dir", []() {
            return paths::home_dir().string();
            });

    lua.set_function("list_dir", [this](const std::string& path) {
            if(!fs::is_directory(path)) return sol::table();

            sol::table table = lua.create_table();
            int index = 1; //fucking lua

            for (const auto& entry : fs::directory_iterator(path)) {  
                table[index] = entry.path().string();  
                index++;  
            }  

            return table;
            });

    lua.script_file(path); //load and run the script
}

//TODO
std::vector<Game> Plugin::find_saves() {
    sol::protected_function fn = lua["find_saves"];
    auto result = fn();
    std::vector<Game> games;

    if (!result.valid()) {
        sol::error err = result;
        get_logger().error("Lua error: {}", err.what());
        return {};
    }
    
    sol::table table = result.get<sol::table>();
    for (auto& [key, val] : table) {
        sol::table entry = val.as<sol::table>();
        Game g;
        g.type = PlatformType::CUSTOM;
        g.game_name = entry["game_name"].get<std::string>();
        g.appid = entry["appid"].get<std::string>();
        g.save_path = entry["save_path"].get<std::string>();
        games.emplace_back(g);
    }
    return games;
}
