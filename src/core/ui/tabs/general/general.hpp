#pragma once
#include "core/globals.hpp"
#include "core/detection/detection.hpp"

struct GeneralTab {
    Detection::DetectionResult* m_result = nullptr;
    const std::unordered_map<std::string, GLuint>* m_textures = nullptr;
    Config* m_config = nullptr;
    TabState* m_state = nullptr;

    std::optional<Detection::DetectionResult> render(const Fonts& fonts, Detection::DetectionResult& result, 
                                                     const std::unordered_map<std::string, GLuint>& texture_id, 
                                                     Config& config, 
                                                     TabState& state);

    void on_result_changed();
    void render_cards();
    void render_card(const Game& primary, const Game& active_game, const std::vector<int>& group, int gi, float image_height);
    void render_modals();

    size_t last_game_count = 0;
    bool open_restore_modal = false, open_delete_modal = false;
    const Game* pending_restore_game = nullptr;
    const Game* pending_delete_game = nullptr;
    std::vector<std::vector<int>> grouped_games;
    std::future<Detection::DetectionResult> refresh_future;

    std::string label_input = "My awesome savegame";

    int spinner_frame = 0;
    const char* spinner = "|/-\\";
    const Fonts* m_fonts = nullptr;
};
