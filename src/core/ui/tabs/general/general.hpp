#pragma once
#include "core/globals.hpp"
#include "core/detection/detection.hpp"

struct GeneralTab {
    void render(const Fonts& fonts, Detection::DetectionResult& result, std::unordered_map<std::string, GLuint> texture_id, Config& config, TabState& state);
    void on_result_changed(Detection::DetectionResult& result);

    size_t last_game_count = 0;

    bool open_restore_modal = false, open_delete_modal = false;
    const Game* pending_restore_game = nullptr;
    const Game* pending_delete_game = nullptr;

    std::vector<std::vector<int>> grouped_games;
    std::unordered_map<std::string, size_t> appid_to_group;

    std::unordered_map<std::string, int> selected_source;
};
