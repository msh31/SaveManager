#pragma once
#include "../base_model.hpp"

/*
    TODO LIST

    1. rename the members, its a bit confusing
*/

class CTagsModal : public CModalBase {
    public:
        CTagsModal( ) : CModalBase( "Manage Tags" ) {};

        void open(
            const std::string& game_name, const fs::path& backup, const std::vector<std::string>& tag_list,
            const std::function<void( const std::string&, const std::vector<std::string>& )>& on_saved );
        void render_content( );

    private:
        // 1.
        std::string m_pending_rename_game = { };
        std::string m_new_tag_input = { };
        std::vector<std::string> m_pending_tags = { };
        fs::path m_pending_rename_backup = { };

        std::function<void( const std::string&, const std::vector<std::string>& )> m_on_saved;
};