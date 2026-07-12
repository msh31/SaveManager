#pragma once
#include <types.hpp>

#include <utils/blacklist/blacklist.hpp>
#include <utils/translations/translations.hpp>

namespace Detection {
    std::vector<Game> find_saves( const Blacklist&, const Translations& );
}; // namespace Detection
