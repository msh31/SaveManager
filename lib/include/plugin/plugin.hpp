#pragma once
#include "types.hpp"
#include <sol/sol.hpp>

class CPlugin {
  public:
    CPlugin( std::filesystem::path path );
    std::vector<Game> find_saves( );

  private:
    sol::state m_lua;
    bool m_show_parent_path = false;
};
