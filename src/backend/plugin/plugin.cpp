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
    return {}; 
}
