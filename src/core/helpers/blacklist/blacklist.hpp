#pragma once
#include "../../external/json.hpp"
#include <unordered_set>

using json = nlohmann::json;

namespace Blacklist {
    void init();
    void save();

    bool is_blacklisted(const std::string& game_name);

    inline std::unordered_set<std::string> blacklisted_games;
}
