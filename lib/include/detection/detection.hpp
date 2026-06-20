#pragma once
#include <types.hpp>

#include "utils/blacklist/blacklist.hpp"

namespace Detection {
    std::vector<Game> find_saves( const Blacklist& );
}; // namespace Detection
