#pragma once
#include <frontend/views/base_view.hpp>

#include <frontend/components/modals/tags/tags_modal.hpp>
#include <frontend/components/modals/restore_conflicts/restore_conflicts.hpp>
#include <frontend/components/modals/backup_preview/backup_preview.hpp>
#include <frontend/components/modals/create_ruleset/create_ruleset.hpp>

/*
    TODO LIST

    1. replace dummy data with real data
*/

class CHomeView : public CBaseView {
    public:
        ~CHomeView( ) override;
        void render( ) override;
        void on_enter( ) override;
        void on_exit( ) override;

    private:
        // 1.
        CTagsModal m_tags_modal; 
        CConflictsModal m_conflicts_modal;
        CBackupPreviewModal m_backup_preview_modal;
        CCreateRulesetModal m_ruleset_modal;
};
