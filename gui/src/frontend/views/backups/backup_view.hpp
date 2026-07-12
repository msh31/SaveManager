#pragma once
#include <config/config.hpp>
#include <detection/detection.hpp>
#include <utils/utils.hpp>

class CBackupsView {
    public:
        CBackupsView( CConfig& config ) : m_config( config ) {};
        void render( const std::vector<Game>& games_snapshot );
        void on_enter( const std::vector<Game>& games_snapshot );
        void on_exit( );

    private:
        CConfig& m_config;

        void render_game_row(
            const BackupEntry& bentry,
            const std::unordered_map<std::string, std::unordered_map<std::string, TagCache>>& labels_cache );
        void render_backup_row(
            fs::path path, const fs::path& save_path, const std::unordered_map<std::string, TagCache>& labels,
            const std::string& game_name );
        void render_modals( );
        void add_new_entry( std::vector<Game> snapshot );

        // UI state
        std::unordered_map<std::string, bool> m_card_collapsed;

        std::mutex m_mutex;

        // Futures
        std::future<void> m_refresh_future;

        // other
        bool m_reload_backups = false;
        std::vector<BackupEntry> m_backups;

        std::unordered_map<std::string, std::unordered_map<std::string, TagCache>> m_labels_cache;

        // Modal state
        std::string m_new_tag_input;
        std::vector<std::string> m_pending_tags;
        bool m_open_tags_modal = false;
        std::string m_pending_rename_game;
        fs::path m_pending_rename_backup;
};
