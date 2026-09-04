#pragma once
#include <frontend/views/base_view.hpp>

#include <frontend/components/modals/tags/tags_modal.hpp>
#include <frontend/components/modals/restore_conflicts/restore_conflicts.hpp>
#include <frontend/components/modals/backup_preview/backup_preview.hpp>
#include <frontend/components/modals/create_ruleset/create_ruleset.hpp>
#include <frontend/components/modals/backup_restore/backup_restore.hpp>

#include <async_queue/async_queue.hpp>

/*
    TODO LIST

    1. move this out once its required ( when porting the backup view )
*/

class CHomeView : public CBaseView {
    public:
        ~CHomeView( ) override;
        void render( ) override;
        void on_enter( ) override;
        void on_exit( ) override;

    private:
        struct SaveFileInfo {
                uintmax_t size;
                fs::file_time_type mtime;
                bool is_dir;
        };

        struct GameCache {
                std::vector<fs::path> save_files;
                std::unordered_map<fs::path, SaveFileInfo> file_info;
                std::unordered_map<fs::path, SaveFileInfo> backup_info;

                int backup_count;

                bool has_conflicts = false;

                bool has_undo = false;
                fs::path undo_path = { };

                std::vector<fs::path> backup_paths;
                 std::unordered_map<std::string, TagCache> tags;
        };

        enum class SortMode { Recent, Alphabetical };
        SortMode m_sort_mode = SortMode::Alphabetical; //default

        std::string m_search_query = { };
        std::optional<std::string> m_platform_filter;

        bool m_focus_search = false;

        uint64_t m_seen_generation = 0; //snapshot counter
        size_t m_filtered_game_count = 0;

        std::vector<Game> m_games_snapshot = { };
        std::vector<std::vector<int>> m_grouped_games = { };
        std::vector<std::string> m_available_platform_labels; // distinct Game::platform_label values currently detected

        std::unordered_map<std::string, GameCache> m_game_cache;
        std::unordered_map<std::string, fs::file_time_type> m_game_last_modified = { };

        std::vector<std::pair<fs::path, fs::path>> m_conflicts = { };

        void render_toolbar( );
        void render_game_list( );
        void render_game_content(
            std::pair<int, int> sb_count, const Game& game, bool has_conflicts,
            std::vector<std::pair<fs::path, const Game*>> files );

        void render_game_row( const std::vector<int>& group, int gi );
        void render_backup_row(
            const fs::path& backup, const Game& game, const std::unordered_map<std::string, TagCache>& labels,
            const SaveFileInfo& info );
        void render_save_row( const fs::path& save_file, const Game& game, const SaveFileInfo& save_info );

        void render_modals( );
        CTagsModal m_tags_modal; 
        CConflictsModal m_conflicts_modal;
        CBackupPreviewModal m_preview_modal;
        CCreateRulesetModal m_ruleset_modal;
        CBackupRestoreModal m_restore_modal;

        std::future<void> m_backup_future;
        std::vector<Game> m_pending_invalidate;

        std::unordered_map<std::string, bool> m_card_collapsed;
        std::unordered_map<std::string, bool> m_backups_expanded;
        std::unordered_map<std::string, bool> m_saves_expanded;

        void refresh_game_state( );
        void invalidate_cache( const std::vector<Game>& games, std::function<void( )> on_done );
        CAsyncQueue m_queue;
};
