#pragma once
class Config;

namespace Detection {
struct DetectionResult {
    std::vector<Game> games;
    std::vector<std::vector<int>> get_grouped() const;
};

void add_game(std::expected<std::vector<Game>, DetectionError> result, const std::string& platform, std::vector<Game>& games);
DetectionResult find_saves(Config& config);
};
