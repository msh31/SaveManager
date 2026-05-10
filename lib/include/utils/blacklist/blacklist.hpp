#pragma once

namespace Blacklist {
    void init();
    void save();

    bool is_blacklisted(const std::string& game_name);

    inline std::unordered_set<std::string> blacklisted_games;
}
