#pragma once
#include "../base_model.hpp"
#include <detection/game.hpp>

/*
    TODO LIST

    1. 
*/

class CBackupRestoreModal : public CModalBase {
    public:
        CBackupRestoreModal( ) : CModalBase( "Restore Backup" ) {};

        void open(
            const Game& game, const fs::path& backup_file, const std::function<void( const Game& )>& on_restored,
            const std::function<void( const Game&, const std::vector<std::pair<fs::path, fs::path>>& )>& on_conflicts );
        void render_content( );

    private:
        Game m_pending_game;
        fs::path m_pending_backup = { };

        std::vector<std::string> m_restore_entries = { };
        std::vector<std::pair<fs::path, fs::path>> m_pending_conflicts = { };
        std::unordered_set<std::string> m_pending_exclusions = { };;
        std::unordered_map<std::string, bool> m_restore_checked = { };

        std::function<void( const Game& )> m_on_restored;
        std::function<void( const Game&, const std::vector<std::pair<fs::path, fs::path>>& )> m_on_conflicts;
};