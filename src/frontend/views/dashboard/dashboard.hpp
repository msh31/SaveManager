#pragma once
#include "backend/detection/detection.hpp"

struct DashboardTab {
    std::vector<std::vector<int>> grouped_games;
    std::unordered_map<std::string, bool> card_collapsed;
    std::unordered_map<std::string, bool> backups_collapsed;
    std::string search_query = "";

    bool open_rename_modal = false;
    const Game* pending_rename_game = nullptr;

    std::future<Detection::DetectionResult> refresh_future;
    std::future<void> backup_future;

    fs::path pending_rename_backup;
    std::string rename_input;
    std::string label_input = "My awesome savegame";
    size_t last_game_count = 0;
    int spinner_frame = 0;

    std::optional<Detection::DetectionResult> render(const Fonts&, Detection::DetectionResult&,
        const std::unordered_map<std::string, GLuint>&, Config&);

private:
    struct RenderContext {
        Detection::DetectionResult& result;
        const std::unordered_map<std::string, GLuint>& textures;
        Config& config;
        const Fonts& fonts;
    };

    enum class SortMode { 
        Recent, 
        Alphabetical 
    };
    SortMode sort_mode = SortMode::Alphabetical;
    std::optional<PlatformType> platform_filter = std::nullopt; // nullopt = all

    std::unordered_map<std::string, fs::file_time_type> game_last_modified;

    void on_result_changed(RenderContext&);
    void render_toolbar(RenderContext&);
    void render_game_list(RenderContext&);
    void render_game_row(RenderContext&, const std::vector<int>& group, int gi);
    void render_save_row(RenderContext& ctx, const fs::path& save_file, const Game& game);
    void render_backup_row(RenderContext& ctx, const fs::path& backup, const Game& game, const std::unordered_map<std::string, std::string>& labels);
    void render_modals(RenderContext&);
};
