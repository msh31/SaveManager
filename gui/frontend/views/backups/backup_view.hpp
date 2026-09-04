#pragma once
#include <detection/detection.hpp>
#include <backup/backup.hpp>
#include <tags/tags.hpp>

#include <async_queue/async_queue.hpp>
#include <frontend/components/modals/tags/tags_modal.hpp>

class CBackupsView {
    public:
        ~CBackupsView( );

        void render( const std::vector<Game>& games_snapshot );
        void on_enter( const std::vector<Game>& games_snapshot );
        void on_exit( );

    private:
        using LabelsCache = std::unordered_map<std::string, std::unordered_map<std::string, TagCache>>;
        using RefreshResult = std::pair<std::vector<BackupEntry>, LabelsCache>;

        void render_game_row( const BackupEntry& bentry, const LabelsCache& labels_cache );

        void render_backup_row(
            fs::path path, const fs::path& save_path, const std::unordered_map<std::string, TagCache>& labels,
            const std::string& game_name );

        void render_modals( );
        void request_refresh( const std::vector<Game>& games_snapshot );
        static RefreshResult scan_backups( const std::vector<Game>& snapshot );

        // UI state
        std::unordered_map<std::string, bool> m_card_collapsed = { };

        CAsyncQueue m_queue;
        bool m_refreshing = false;

        // other
        bool m_reload_backups = false;
        std::vector<BackupEntry> m_backups;

        LabelsCache m_labels_cache;

        CTagsModal m_tags_modal;
};
