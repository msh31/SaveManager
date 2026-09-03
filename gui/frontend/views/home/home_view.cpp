#include "home_view.hpp"
#include <frontend/notification/notification.hpp>
#include <utils/utils.hpp>

void CHomeView::on_enter( ) {
    
}

void CHomeView::render( ) {
    m_tags_modal.render();

    // 1. 
    if ( ImGui::Button( "Tags" ) ) {
        Game game;
        game.game_name = "SaveManager";
        fs::path backup = "test_backup.zip";
        std::vector<std::string> tags = { "endgame", "100%" };

        m_tags_modal.open( game, backup, tags, []( const std::string& filename, const std::vector<std::string>& tags ) {
            Notify::show_notification( "Tags", "Added tags!", 2000 );
        } );
    }
}

void CHomeView::on_exit( ) {
    
}

CHomeView::~CHomeView( ) {}
