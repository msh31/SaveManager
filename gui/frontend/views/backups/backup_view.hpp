#pragma once
#include <backup/backup.hpp>
#include <detection/detection.hpp>
#include <tags/tags.hpp>

#include <async_queue/async_queue.hpp>
#include <frontend/components/modals/tags/tags_modal.hpp>
#include <frontend/components/modals/restore_conflicts/restore_conflicts.hpp>
#include <frontend/components/modals/backup_restore/backup_restore.hpp>
#include <frontend/components/modals/backup_preview/backup_preview.hpp>

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
            fs::path path, const std::vector<fs::path>& save_paths,
            const std::unordered_map<std::string, TagCache>& labels, const Game& game );

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
        CConflictsModal m_conflicts_modal;
        CBackupRestoreModal m_restore_modal;
        CBackupPreviewModal m_preview_modal; //todo
};
