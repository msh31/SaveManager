#pragma once
#include "../../external/json.hpp"

using json = nlohmann::json;

namespace translations {
    void init();
    std::optional<std::string> get_game_name_rsg(std::string_view folder_name);
    std::optional<std::string> get_game_name_ubi(const std::string& game_id);
    std::optional<std::string> get_steam_id(const std::string& game_name);
    std::optional<std::string> get_steam_name(const std::string& appid);
}
