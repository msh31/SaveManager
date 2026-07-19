#pragma once
#include <config/config.hpp>
#include <detection/detection.hpp>
#include <features/features.hpp>

#include <frontend/views/backups/backup_view.hpp>
#include <frontend/views/base_view.hpp>

#include <backend/task_runner/task_runner.hpp>
#include <utils/steam/steam.hpp>
#include <utils/unreal_name_cache/unreal_name_cache.hpp>

class CDashboardView : public CBaseView {
    public:
        CDashboardView(
            CConfig& config, const Blacklist& blacklist, const Translations& translations,
            const SteamManifestCache& manifest_cache, UnrealNameCache& name_cache )
            : m_config( config ), m_backups_view( config ), m_blacklist( blacklist ), m_translations( translations ),
              m_manifest_cache( manifest_cache ), m_name_cache( name_cache ) {};
        ~CDashboardView( ) override;
        void render( ) override;
        void on_enter( ) override;
        void on_exit( ) override;

    private:
        void on_result_changed( );
        void invalidate_cache( const std::vector<Game>& games, std::function<void( )> on_done = nullptr );

        void render_toolbar( );
        void render_game_list( );
        void render_game_content(
            std::pair<int, int> sb_count, const Game& game, bool has_conflicts,
            std::vector<std::pair<fs::path, const Game*>> files );
        void render_game_row( const std::vector<int>& group, int gi );
        void render_save_row( const fs::path& save_file, const Game& game );
        void render_backup_row(
            const fs::path& backup, const Game& game, const std::unordered_map<std::string, TagCache>& labels );
        void render_modals( );

        CConfig& m_config;
        std::vector<Game> m_result;
        CBackupsView m_backups_view;
        const Blacklist& m_blacklist;
        const Translations& m_translations;
        const SteamManifestCache& m_manifest_cache;
        UnrealNameCache& m_name_cache;

        CTaskRunner m_task_runner;

        // Detection / cache
        struct GameCache {
                std::vector<fs::path> save_files;
                int backup_count;
                bool has_conflicts = false;
                std::vector<fs::path> backup_paths;

                std::unordered_map<std::string, TagCache> tags;
        };

        std::mutex m_result_mutex;
        std::vector<Game> m_games_snapshot;
        std::vector<std::vector<int>> m_grouped_games;

        std::unordered_map<std::string, GameCache> m_game_cache;
        std::unordered_map<std::string, fs::file_time_type> m_game_last_modified;

        size_t m_last_game_count = 0;

        // UI state
        enum class SortMode { Recent, Alphabetical };

        std::string m_search_query;
        std::optional<std::string> m_platform_filter;
        std::vector<std::string> m_available_platform_labels; // distinct Game::platform_label values currently detected
        SortMode m_sort_mode = SortMode::Alphabetical;
        bool m_focus_search = false;
        size_t m_filtered_game_count = 0;

        std::unordered_map<std::string, bool> m_card_collapsed;
        std::unordered_map<std::string, bool> m_backups_expanded;

        float m_detection_duration = 0.0f;
        bool m_backups_tab_was_active = false;

        std::chrono::time_point<std::chrono::steady_clock> m_detection_start_time;

        // Model state
        bool m_open_tags_modal = false;
        std::string m_new_tag_input;
        std::vector<std::string> m_pending_tags;
        Game m_pending_rename_game{ };
        fs::path m_pending_rename_backup;
        bool m_open_conflict_modal = false;
        std::vector<std::pair<fs::path, fs::path>> m_pending_conflicts;
        Game m_pending_conflict_game{ };

        bool m_open_restore_modal = false;
        std::unordered_set<std::string> m_pending_exclusions = { };
        fs::path m_pending_restore_backup = { };
        Game m_game_exclusions_restore; // horrible name
        std::vector<std::string> m_restore_entries;
        std::unordered_map<std::string, bool> m_restore_checked = { };

        // Futures
        std::future<void> m_refresh_future;
        std::future<void> m_backup_future;
        std::vector<Game> m_pending_invalidate;
};
