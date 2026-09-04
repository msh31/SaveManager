#include "backup_restore.hpp"
#include <utils/utils.hpp>
#include <frontend/notification/notification.hpp>
#include <backup/backup.hpp>

void CBackupRestoreModal::open(
    const Game& game, const fs::path& backup_file, const std::function<void( const Game& )>& on_restored,
    const std::function<void( const Game&, const std::vector<std::pair<fs::path, fs::path>>& )>& on_conflicts ) {

    m_restore_checked.clear( );
    m_pending_game = game;
    m_pending_backup = backup_file;
    m_on_restored = on_restored;
    m_on_conflicts = on_conflicts;

     auto res_entries = Backup::get_backup_entries( m_pending_backup );
     if ( res_entries.empty( ) ) {
         Notify::show_notification( "Restore Failed", "Found no entries in backup, odd.", 2000 );
         return;
     }
     m_restore_entries = res_entries;

     // select all by default
    for ( const auto& e : m_restore_entries ) {
        m_restore_checked[e] = true;
    }

    request_open( );
}

void CBackupRestoreModal::render_content( ) {
    ImGui::TextWrapped(
        "Select all files you would like to restore from %s",
        utils::path_to_utf8( m_pending_backup.filename( ) ).c_str( ) );

    //if ( ImGui::Button( "Select All" ) ) {
    //    m_restore_checked[entry] = !m_restore_checked[entry];
    //}
    auto height = std::min( m_restore_entries.size( ) * ImGui::GetFrameHeightWithSpacing( ) + 1.5f, 400.0f );
    ImGui::BeginChild( "##Restore entries", ImVec2( 650, height ) );
    for ( const auto& entry : m_restore_entries ) {
        ImGui::PushID( entry.c_str( ) );
        std::string text = std::format( "Include '{}'?", utils::path_to_utf8( utils::utf8_to_path( entry ).filename( ) ) );
        ImGui::Checkbox( text.c_str( ), &m_restore_checked[entry] );
        ImGui::SetItemTooltip( "%s", entry.c_str( ) );
        ImGui::Separator( );
        ImGui::PopID( );
    }
    ImGui::EndChild( );

    ImGui::Separator( );

    if ( ImGui::Button( "Restore" ) ) {
        m_pending_exclusions.clear( );
        for ( const auto& entry : m_restore_checked ) {
            if ( entry.second == false ) {
                m_pending_exclusions.insert( entry.first );
            }
        }

        if ( Backup::restore_backup(
                 m_pending_backup, m_pending_game.save_paths, m_pending_conflicts,
                 m_pending_exclusions ) ) {
            if ( m_pending_conflicts.empty( ) ) {
                auto str = std::format(
                    "Successfully restored a backup for: {}", utils::path_to_utf8( m_pending_backup.filename( ) ) );
                Notify::show_notification( "Backup Restored!", str, 2000 );
                m_on_restored( m_pending_game );
                ImGui::CloseCurrentPopup( );
            } else {
                m_on_conflicts( m_pending_game, m_pending_conflicts );
                ImGui::CloseCurrentPopup( );
                Notify::show_notification(
                    "Backup Restore Failed!", "Failed to restore backup due to conflicts, resolve them!", 2000 );
            }
        } else {
            Notify::show_notification( "Restore", "Failed to restore backup!", 2000 );
            ImGui::CloseCurrentPopup( );
        }
    }

    ImGui::SameLine( );

    if ( ImGui::Button( "Cancel" ) ) ImGui::CloseCurrentPopup( );
}