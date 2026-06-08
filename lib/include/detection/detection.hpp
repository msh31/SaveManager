#pragma once
#include <types.hpp>

class CConfig;

namespace Detection {
    void find_saves( CConfig& config, std::vector<Game>& games );

    inline std::vector<std::future<std::expected<std::vector<Game>, SMError>>> detection_futures;
}; // namespace Detection
