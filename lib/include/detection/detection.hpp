#pragma once
#include <types.hpp>

class Config;

namespace Detection {
struct DetectionResult {
    std::vector<Game> games;
    std::vector<std::vector<int>> get_grouped( ) const;
    std::shared_mutex d_mutex;
};

void add_game( std::expected<std::vector<Game>, DetectionError> result, const std::string &platform,
               DetectionResult &d_result );
void find_saves( Config &config, DetectionResult &d_result );
}; // namespace Detection
