#pragma once
#include <types.hpp>

class CConfig;
class CLudusaviParser;

namespace Detection {
    void
    add_game( std::expected<std::vector<Game>, SMError> result, const std::string& platform, std::vector<Game>& games );
    void find_saves( CConfig& config, std::vector<Game>& games );

    // should not be called externally
    void find_saves_ludusavi( CConfig& config, std::vector<Game>& games, std::shared_ptr<CLudusaviParser>& parser );
}; // namespace Detection
