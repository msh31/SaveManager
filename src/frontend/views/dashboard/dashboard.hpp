#pragma once
#include "globals.hpp"
#include "backend/detection/detection.hpp"

struct DashboardTab {
    std::vector<std::vector<int>> grouped_games;
    std::unordered_map<std::string, bool> card_collapsed;
    std::unordered_map<std::string, bool> backups_collapsed;
    std::string search_query = "";

    bool open_restore_modal = false;
    bool open_delete_modal = false;
    const Game* pending_restore_game = nullptr;
    const Game* pending_delete_game = nullptr;

    std::future<Detection::DetectionResult> refresh_future;
    std::future<void> backup_future;

    std::string label_input = "My awesome savegame";
    size_t last_game_count = 0;
    int spinner_frame = 0;

    std::optional<Detection::DetectionResult> render(const Fonts&, Detection::DetectionResult&,
        const std::unordered_map<std::string, GLuint>&, Config&, TabState&);

private:
    struct RenderContext {
        Detection::DetectionResult& result;
        const std::unordered_map<std::string, GLuint>& textures;
        Config& config;
        TabState& state;
        const Fonts& fonts;
    };

    void on_result_changed(RenderContext&);
    void render_toolbar(RenderContext&);
    void render_game_list(RenderContext&);
    void render_game_row(RenderContext&, const std::vector<int>& group, int gi);
    void render_save_row(RenderContext& ctx, const fs::path& save_file, const Game& game);
    void render_backup_row(RenderContext& ctx, const fs::path& backup);
    void render_modals(RenderContext&);
};
