#include "debug_view.hpp"
#include <frontend/dialogs/confirm/confirm_dialog.hpp>
#include <frontend/notification/notification.hpp>

#include <utils/ludisavi_parser/ludusavi_parser.hpp>

#include <utils/steam/steam.hpp>

void CDebugView::render( ) {
    m_task_runner.update( ); // needs a refactor but fine for now

    if ( ImGui::Button( "Click Me" ) ) {
        Notify::show_notification( "", "Click Me button has been clicked!", 2000 );
    }
    ImGui::Separator( );
    if ( ImGui::Button( "  \xef\x80\x81  Test Icon" ) ) {
        Notify::show_notification( "", "Icon button has been clicked!", 2000 );
    }

    ImGui::Text( "\xef\x80\x88 \xef\x80\xad \xef\x83\xa9" );

    if ( ImGui::Button( "Test Notification" ) ) {
        Notify::show_notification( "I am a title", "I am the body", 2000 );
    }
    ImGui::SameLine( );
    if ( ImGui::Button( "Test confirm dialog" ) ) {
        ConfirmDialog::show( "Are you sure?", [this] { Notify::show_notification( "Test", "This is a test", 1000 ); } );
    }
    ImGui::SameLine( );
    if ( ImGui::Button( "Test async task" ) ) {
        m_task_runner.run(
            [] { std::this_thread::sleep_for( std::chrono::seconds( 2 ) ); },
            [] { Notify::show_notification( "Async", "Task complete!", 2000 ); } );
    }

    if ( ImGui::Button( "Test manifest" ) ) {
        m_task_runner.run(
            [] {
                CLudusaviParser               m_parser;
                std::vector<ManifestSavePath> test = m_parser.get_save_paths( 3768760 ); // test
                for ( const auto& entry : test ) {
                    SPDLOG_INFO(
                        "path: {} resolved: {} linux: {}", entry.unresolved_path.string( ),
                        entry.resolved_path.string( ), entry.is_linux );
                }
            },
            [] { Notify::show_notification( "Async", "Manifest test complete!", 2000 ); } );
    }
    ImGui::SameLine( );
    if ( ImGui::Button( "Steam UserID test" ) ) {
        SPDLOG_INFO( SteamHelper::parse_steam_userid( ).value_or( "no user id found" ) );
    }
}

void CDebugView::on_enter( ) {};
void CDebugView::on_exit( ) {}
CDebugView::~CDebugView( ) {}
