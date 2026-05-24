#pragma once
#include <config/config.hpp>
#include <detection/detection.hpp>
#include <frontend/views/base_view.hpp>

class CDashboardView : public CBaseView {
    public:
        CDashboardView( CConfig& config ) : m_config( config ) {};
        ~CDashboardView( ) override;
        void render( ) override;
        void on_enter( ) override;
        void on_exit( ) override;

    private:
        void on_result_changed( );
        void render_toolbar( );
        void render_game_list( );
        void render_game_row( const std::vector<int>& group, int gi );
        void render_save_row( const fs::path& save_file, const Game& game );
        void render_backup_row(
            const fs::path& backup, const Game& game, const std::unordered_map<std::string, std::string>& labels );
        void render_modals( );

        CConfig& m_config;

        // Detection / cache
        struct GameCache {
                std::vector<fs::path>                        save_files;
                int                                          backup_count;
                std::unordered_map<std::string, std::string> labels;
                bool                                         has_conflicts = false;
        };

        Detection::DetectionResult                          m_result;
        std::vector<std::vector<int>>                       m_grouped_games;
        std::unordered_map<std::string, GameCache>          m_game_cache;
        std::unordered_map<std::string, fs::file_time_type> m_game_last_modified;
        size_t                                              m_last_game_count = 0;

        // UI state
        enum class SortMode { Recent, Alphabetical };

        std::string                           m_search_query;
        std::optional<PlatformType>           m_platform_filter;
        SortMode                              m_sort_mode    = SortMode::Alphabetical;
        bool                                  m_focus_search = false;
        std::unordered_map<std::string, bool> m_card_collapsed;
        std::unordered_map<std::string, bool> m_backups_collapsed;

        // Model state
        bool                                       m_open_conflict_modal = false;
        bool                                       m_open_rename_modal   = false;
        bool                                       m_open_schedule_modal = false;
        std::string                                m_rename_input;
        Game                                       m_pending_rename_game{ };
        fs::path                                   m_pending_rename_backup;
        std::vector<std::pair<fs::path, fs::path>> m_pending_conflicts;

        // Futures
        std::future<void> m_refresh_future;
        std::future<void> m_backup_future;

        // TODO: implement schedule usage from the lib
        //  Game pending_schedule_game;
        //  std::vector<fs::path> scheduled_files;
        //  std::unordered_set<fs::path> scheduled_files_selected;
        //  bool schedule_enabled = false;
        //  int schedule_interval_hours = 1;
};
