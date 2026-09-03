#pragma once
#include "../base_model.hpp"
#include <detection/game.hpp>

class CCreateRulesetModal : public CModalBase {
    public:
        CCreateRulesetModal( ) : CModalBase( "Create ruleset" ) {};

        void open( const Game& game, const std::function<void( const Game& )>& on_create );
        void render_content( );

    private:
        std::vector<fs::path> m_file_list_ignore_rulset = { };
        std::string m_new_ruleset_text = { };
        fs::path m_pending_ignore_save_root = { };
        fs::path m_ignore_file = { };
        Game m_pending_ignore_game = { };

        std::function<void( const Game& )> m_on_create;
};