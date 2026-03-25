#pragma once
#include "../../external/json.hpp"

using json = nlohmann::json;

namespace CustomGamesFile {
    void init();
    void save();

    struct CustomGame {
        std::string game_name;
        std::string save_path;
        std::string appid;
    };

    inline std::vector<CustomGame> games;
}
