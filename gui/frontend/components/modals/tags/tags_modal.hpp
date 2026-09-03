#pragma once
#include "../base_model.hpp"
#include <sm_error.hpp>
#include <detection/game.hpp>

/*
    TODO LIST

    1. rename the members, its a bit confusing
    2. move this out of the GUI project to the lib someplace
*/

class CTagsModal : public CModalBase {
    public:
        CTagsModal(  ) : CModalBase("Manage Tags") {};

        void open(
            const Game& game, const fs::path& backup, const std::vector<std::string>& tag_list,
            const std::function<void( const std::string&, const std::vector<std::string>& )>& on_saved );
        void render_content( );

    private: 
        //1.
        Game m_pending_rename_game{ };
        std::string m_new_tag_input = { };
        std::vector<std::string> m_pending_tags = { };
        fs::path m_pending_rename_backup = { };

        std::function<void( const std::string&, const std::vector<std::string>& )> m_on_saved;

        //2.
        std::unordered_map<std::string, std::vector<std::string>> load_tags( const std::string& game );
        std::expected<bool, SMError> save_tags(const std::string& game, const std::string& filename, const std::vector<std::string>& tags );
};