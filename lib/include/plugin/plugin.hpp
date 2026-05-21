#pragma once
#include "types.hpp"
#include <sol/sol.hpp>

class Plugin {
  public:
    Plugin( std::filesystem::path path );
    std::vector<Game> find_saves( );

  private:
    sol::state lua;
    bool show_parent_path = false;
};
