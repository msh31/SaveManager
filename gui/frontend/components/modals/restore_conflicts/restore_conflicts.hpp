#pragma once
#include "../base_model.hpp"
#include <detection/game.hpp>

/*
    TODO LIST

    1. 
*/

class CConflictsModal : public CModalBase {
    public:
        CConflictsModal( ) : CModalBase( "Resolve conflict(s)" ) {};

        void open(
            const Game& game, const std::vector<std::pair<fs::path, fs::path>>& conflicts, 
            const std::function<void( const Game& )>& on_resolved );
        void render_content( );

    private: 
        std::vector<std::pair<fs::path, fs::path>> m_pending_conflicts = { };
        Game m_conflicted_game = { };

        std::function<void( const Game& )> m_on_resolved;
};