#pragma once
#include "types.hpp"
#include <sol/sol.hpp>

class CPlugin {
    public:
        CPlugin( std::filesystem::path path );
        std::vector<Game> find_saves( );

    private:
        // TODO: use a shared sol::state with per-plugin sandboxed environments instead of one VM per plugin
        sol::state m_lua;
        bool m_show_parent_path = false;
};
