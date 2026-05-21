#pragma once
#include "detection/detection.hpp"
#include <features/scheduler/scheduler.hpp>
#include <utils/utils.hpp>
#include <utils/zip_archive/zip_archive.hpp>

namespace fs = std::filesystem;

struct DashboardTab {
    std::vector<std::vector<int>> grouped_games;
    std::unordered_map<std::string, bool> card_collapsed;
    std::unordered_map<std::string, bool> backups_collapsed;
    std::string search_query = "";

    bool open_rename_modal = false;
    Game pending_rename_game = { };

    bool open_schedule_modal = false;

    std::future<void> refresh_future;
    std::future<void> backup_future;

    fs::path pending_rename_backup;
    std::string rename_input;
    size_t last_game_count = 0;
    int spinner_frame = 0;

    void render( const Fonts &, Detection::DetectionResult &, Config &, SaveScheduler &scheduler );

  private:
    struct RenderContext {
        Detection::DetectionResult &result;
        Config &config;
        const Fonts &fonts;
        const std::vector<Game> &games;
        SaveScheduler &scheduler;
    };

    struct GameCache {
        std::vector<fs::path> save_files;
        int backup_count;
        std::unordered_map<std::string, std::string> labels;
        bool has_conflicts = false;
    };
    std::unordered_map<std::string, GameCache> game_cache;

    enum class SortMode { Recent, Alphabetical };
    SortMode sort_mode = SortMode::Alphabetical;
    std::optional<PlatformType> platform_filter = std::nullopt; // nullopt = all
    std::unordered_map<std::string, fs::file_time_type> game_last_modified;

    void on_result_changed( RenderContext & );
    void render_toolbar( RenderContext & );
    void render_game_list( RenderContext & );
    void render_game_row( RenderContext &, const std::vector<int> &group, int gi );
    void render_save_row( RenderContext &ctx, const fs::path &save_file, const Game &game );
    void render_backup_row( RenderContext &ctx, const fs::path &backup, const Game &game,
                            const std::unordered_map<std::string, std::string> &labels );
    void render_modals( RenderContext & );

    bool focus_search = false;

    std::vector<std::pair<fs::path, fs::path>> pending_conflicts;
    bool open_conflict_modal = false;

    Game pending_schedule_game;
    std::vector<fs::path> scheduled_files;
    std::unordered_set<fs::path> scheduled_files_selected;
    bool schedule_enabled = false;
    int schedule_interval_hours = 1;
};
