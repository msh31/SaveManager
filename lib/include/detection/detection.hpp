#pragma once
#include <types.hpp>

class CConfig;

namespace Detection {
    struct DetectionResult {
            std::vector<Game> games;
            std::shared_mutex d_mutex;
    };

    void add_game(
        std::expected<std::vector<Game>, DetectionError> result, const std::string& platform,
        DetectionResult& d_result );
    void find_saves( CConfig& config, DetectionResult& d_result );
}; // namespace Detection
