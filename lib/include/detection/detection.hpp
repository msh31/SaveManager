#pragma once
#include <types.hpp>

class CConfig;

namespace Detection {
    void find_saves( CConfig& config, std::vector<Game>& games );

    // should not be called externally
    void
    add_game( std::expected<std::vector<Game>, SMError> result, const std::string& platform, std::vector<Game>& games );
}; // namespace Detection
