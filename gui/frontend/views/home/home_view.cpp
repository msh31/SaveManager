#include "home_view.hpp"
#include <frontend/notification/notification.hpp>
#include <utils/utils.hpp>

/*
    TODO LIST

    1. replace dummy data with real data
*/

void CHomeView::on_enter( ) {
    
}

void CHomeView::render( ) {
    m_tags_modal.render();
    m_conflicts_modal.render( );
    m_backup_preview_modal.render( );
    m_ruleset_modal.render( );

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
    ImGui::SameLine( );
    if ( ImGui::Button( "Conflicts" ) ) {
        Game game;
        game.game_name = "SaveManager";
        std::vector<std::pair<fs::path, fs::path>> conflicts = { };
        
        for ( int i = 0; i < 9; i++ ) {
            std::string pstr = std::format( "{}.zip", i );
            fs::path p = fs::temp_directory_path( ) / pstr;
            conflicts.push_back( { p, p } );
        }
             
        m_conflicts_modal.open( game, conflicts, []( const Game& g ) {} );
    }
    ImGui::SameLine( );
    if ( ImGui::Button( "Preview" ) ) {
        std::vector<std::string> list = { "test.zip", "more_test.zip", "testy_test.zip", "cool_test.zip", "even_cooler_test.zip" };
        m_backup_preview_modal.open( list );
    }
    ImGui::SameLine( );
    if ( ImGui::Button( "Create Ruleset" ) ) {
        std::error_code ec;
        auto ruleset_demo_path = fs::temp_directory_path( ec ) / "SaveManager-ruleset-demo";
        if ( ec || ( !fs::create_directories( ruleset_demo_path, ec ) && ec ) ) {
            Notify::show_notification( "Ignore ruleset", "Failed to create the ruleset demo directory!", 2000 );
            return;
        }

        Game game;
        game.game_name = "SaveManager";
        game.save_paths = { std::move( ruleset_demo_path ) };
        m_ruleset_modal.open( game, []( const Game& g ) {} );
    }
}

void CHomeView::on_exit( ) {
    
}

CHomeView::~CHomeView( ) {}
