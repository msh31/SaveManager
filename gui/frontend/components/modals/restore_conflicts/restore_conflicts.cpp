#include "restore_conflicts.hpp"
#include <utils/utils.hpp>
#include <frontend/notification/notification.hpp>


void CConflictsModal::open( const Game& game, const std::vector<std::pair<fs::path, fs::path>>& conflicts,
    const std::function<void( const Game& )>& on_resolved ) {
    m_pending_conflicts = conflicts;
    m_conflicted_game = game;
    m_on_resolved = on_resolved;
    request_open( );
}

void CConflictsModal::render_content( ) {
    std::vector<int> to_remove = { };

    for ( size_t i{ }; i < m_pending_conflicts.size( ); i++ ) {
        ImGui::Text( "%s", utils::path_to_utf8( m_pending_conflicts[i].second.filename( ) ).c_str( ) );

        ImGui::PushID( i );
        if ( ImGui::Button( "Keep" ) ) {
            std::error_code ec;
            fs::rename( m_pending_conflicts[i].second, m_pending_conflicts[i].first, ec );
            if ( ec ) {
                auto str = std::format(
                    "Failed to rename conflict! skipping: {}: {}", m_pending_conflicts[i].first.string( ),
                    ec.message( ) );
                Notify::show_notification( "Conflict Rename failure!", str, 3000 );
            } else {
                to_remove.push_back( i );
            }
        }
        ImGui::PopID( );
        ImGui::SetItemTooltip( "Overwrite the newer save with this restored from backup file" );
        ImGui::SameLine( );
        ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.8f, 0.2f, 0.2f, 1.0f ) );
        ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 0.9f, 0.3f, 0.3f, 1.0f ) );
        ImGui::PushID( i );
        if ( ImGui::Button( "Delete" ) ) {
            fs::remove( m_pending_conflicts[i].second );
            to_remove.push_back( i );
        }
        ImGui::PopID( );
        ImGui::PopStyleColor( 2 );
    }

    for ( int i = to_remove.size( ) - 1; i >= 0; i-- ) {
        m_pending_conflicts.erase( m_pending_conflicts.begin( ) + to_remove[i] );
    }
    
    if ( ImGui::Button( "Cancel" ) ) {
        ImGui::CloseCurrentPopup( );
    }

     // invalidates the cache in the home view
    if ( m_pending_conflicts.empty( ) ) {
        m_on_resolved( m_conflicted_game );
        ImGui::CloseCurrentPopup( );
    }
}