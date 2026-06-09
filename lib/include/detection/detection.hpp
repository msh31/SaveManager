#pragma once
#include <types.hpp>

namespace Detection {
    void find_saves( std::vector<Game>& games );

    inline std::vector<std::future<std::expected<std::vector<Game>, SMError>>> detection_futures;
}; // namespace Detection
