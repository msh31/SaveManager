#include "backup_view.hpp"
#include <backend/font_manager/font_manager.hpp>
#include <features/backup/backup.hpp>
#include <frontend/components/spinner.hpp>
#include <frontend/notification/notification.hpp>
#include <utils/paths.hpp>
#include <utils/utils.hpp>

void CBackupsView::on_enter( const std::vector<Game>& games_snapshot ) {
    if ( m_backups.empty( ) || m_reload_backups ) {
        m_reload_backups = false;
        m_refresh_future = std::async( std::launch::async, [this, games_snapshot] { add_new_entry( games_snapshot ); } );
    }
}

void CBackupsView::on_exit( ) {}

void CBackupsView::add_new_entry( std::vector<Game> snapshot ) {
    std::unordered_map<std::string, fs::path> save_path_lookup;

    std::vector<BackupEntry> backups = { };

    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> labels_cache;

    for ( const auto& game : snapshot ) {
        for ( const auto& save : game.save_paths ) {
            save_path_lookup[sanitize_filename( game.game_name )] = save;
        }
    }

    backups.clear( );

    for ( const auto& entry : fs::directory_iterator( paths::backup_dir( ) ) ) {
        if ( !entry.is_directory( ) ) continue;

        BackupEntry bentry;
        bentry.name = entry.path( ).filename( ).string( );

        labels_cache[bentry.name.string( )] = Features::load_labels( bentry.name.string( ) );

        if ( auto it = save_path_lookup.find( bentry.name.string( ) ); it != save_path_lookup.end( ) )
            bentry.save_path = it->second;

        for ( const auto& entry_b : fs::directory_iterator( entry ) ) {
            if ( entry_b.path( ).extension( ) != ".zip" ) continue;
            bentry.entries.push_back( entry_b.path( ) );
            bentry.size += fs::file_size( entry_b.path( ) );
        }

        if ( bentry.entries.empty( ) ) continue;
        backups.push_back( bentry );
    }
    std::lock_guard lock_em( m_mutex );
    m_labels_cache = std::move( labels_cache );
    m_backups      = std::move( backups );
}

void CBackupsView::render( const std::vector<Game>& games_snapshot ) {
    ImGui::BeginChild(
        "##backup_view", ImVec2( 0, ImGui::GetContentRegionAvail( ).y ), false, ImGuiWindowFlags_NoBackground );

    bool is_refreshing = m_refresh_future.valid( ) &&
                         m_refresh_future.wait_for( std::chrono::seconds( 0 ) ) != std::future_status::ready;

    if ( m_refresh_future.valid( ) && !is_refreshing ) m_refresh_future.get( );

    if ( !is_refreshing && m_reload_backups ) {
        m_reload_backups = false;
        m_refresh_future = std::async( std::launch::async, [this, games_snapshot] { add_new_entry( games_snapshot ); } );
    }

    std::vector<BackupEntry> snapshot;
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> labels_cache;
    {
        std::lock_guard lock( m_mutex );
        snapshot     = m_backups;
        labels_cache = m_labels_cache;
    }

    if ( is_refreshing ) {
        Spinner::render( );
    } else {
        if ( ImGui::Button( "Refresh" ) )
            m_refresh_future = std::async( std::launch::async, [this, games_snapshot] { add_new_entry( games_snapshot ); } );
        ImGui::SetItemTooltip( "Rescans the backups directory" );

        ImGui::Dummy( ImVec2( 0, 5.0f ) );

        for ( const auto& entry : snapshot ) {
            render_game_row( entry, labels_cache );
            ImGui::Dummy( ImVec2( 0, 6.0f ) );
        }
    }

    render_modals( );
    ImGui::EndChild( );
}

void CBackupsView::render_game_row(
    const BackupEntry&                                                                    bentry,
    const std::unordered_map<std::string, std::unordered_map<std::string, std::string>>& labels_cache ) {
    bool& not_collapsed = m_card_collapsed[bentry.name.string( )];

    auto        selectable_id = std::format( "##backup_game_{}", bentry.name.string( ) );
    std::string right_text    = std::format( "{} backups", bentry.entries.size( ) );

    ImGui::PushStyleColor( ImGuiCol_Border, ImVec4( 198 / 255.f, 97 / 255.f, 63 / 255.f, 1.f ) );
    ImGui::PushStyleVar( ImGuiStyleVar_ChildRounding, 4.0f );
    ImGui::BeginChild( selectable_id.c_str( ), ImVec2( 0, 0 ), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY );
    ImGui::PopStyleColor( );

    if ( ImGui::Selectable( "##header", false, ImGuiSelectableFlags_None, ImVec2( 0, 30 ) ) )
        not_collapsed = !not_collapsed;
    ImGui::SameLine( 8.0f );

    ImGui::PushFont( CFontManager::get( ).get_font( "jbm_bold" ).value_or( nullptr ) );
    ImGui::TextColored( ImColor( 198, 97, 63 ).Value, "%s", not_collapsed ? "▼" : "▶" );
    ImGui::PopFont( );
    ImGui::SameLine( );

    ImGui::PushFont( CFontManager::get( ).get_font( "jbm_med" ).value_or( nullptr ) );
    ImGui::Text( "%s", bentry.name.string( ).c_str( ) );
    ImGui::PopFont( );

    ImGui::SameLine( ImGui::GetContentRegionMax( ).x - ImGui::CalcTextSize( right_text.c_str( ) ).x );
    ImGui::Text( "%s", right_text.c_str( ) );

    if ( not_collapsed ) {
        static const std::unordered_map<std::string, std::string> empty_labels;
        auto                                                       it = labels_cache.find( bentry.name.string( ) );
        const auto& labels = ( it != labels_cache.end( ) ) ? it->second : empty_labels;
        for ( const auto& entry : bentry.entries )
            render_backup_row( entry, bentry.save_path, labels, bentry.name.string( ) );
    }

    ImGui::EndChild( );
    ImGui::PopStyleVar( );
}

void CBackupsView::render_backup_row(
    fs::path path, const fs::path& save_path, const std::unordered_map<std::string, std::string>& labels,
    const std::string& game_name ) {
    if ( path.filename( ) == "undo.zip" ) return;
    ImGui::PushID( path.string( ).c_str( ) );
    if ( !fs::exists( path ) ) {
        ImGui::PopID( );
        return;
    }

    auto        it      = labels.find( path.filename( ).string( ) );
    std::string display = ( it != labels.end( ) ) ? it->second : path.filename( ).string( );

    std::string date_text   = std::format( "{:%d/%m/%y %H:%M} | ", fs::last_write_time( path ) );
    float       date_width  = ImGui::CalcTextSize( date_text.c_str( ) ).x;
    auto        b_size      = fs::file_size( path ) / 1024;
    std::string size_text   = std::format( "{}KB  ", b_size );
    float       size_width  = ImGui::CalcTextSize( size_text.c_str( ) ).x;
    float       spacing     = ImGui::GetStyle( ).ItemSpacing.x;
    float       total_width = date_width + size_width + 80.0f * 3 + spacing * 5;

    ImGui::Text( "%s", display.c_str( ) );
    ImGui::SameLine( ImGui::GetContentRegionMax( ).x - total_width );

    ImGui::TextDisabled( "%s", date_text.c_str( ) );
    ImGui::SameLine( 0.0f, spacing );
    ImGui::TextDisabled( "%s", size_text.c_str( ) );
    ImGui::SameLine( 0.0f, spacing );

    ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 3.0f, 3.0f ) );

    if ( ImGui::Button( "Restore", ImVec2( 80.0f, 0 ) ) ) {
        if ( save_path.empty( ) ) {
            Notify::show_notification( "Restore", "Cannot restore: save location unknown.", 2000 );
        } else {
            std::vector<std::pair<fs::path, fs::path>> conflicts;
            Features::restore_backup( path, save_path, conflicts );
        }
    }
    ImGui::SetItemTooltip( "Restore save from backup" );
    ImGui::SameLine( 0.0f, spacing );

    if ( ImGui::Button( "Rename", ImVec2( 80.0f, 0 ) ) ) {
        m_pending_rename_game   = game_name;
        m_pending_rename_backup = path;
        m_rename_input          = ( it != labels.end( ) ) ? it->second : "";
        m_open_rename_modal     = true;
    }
    ImGui::SetItemTooltip( "Rename this backup" );
    ImGui::SameLine( 0.0f, spacing );

    ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.8f, 0.2f, 0.2f, 1.0f ) );
    ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 0.9f, 0.3f, 0.3f, 1.0f ) );
    if ( ImGui::Button( "Delete", ImVec2( 80.0f, 0 ) ) ) {
        if ( fs::remove( path ) ) {
            auto mutable_labels = labels;
            mutable_labels.erase( path.filename( ).string( ) );
            Features::save_labels( game_name, mutable_labels );
            m_reload_backups = true;
            Notify::show_notification( "Backup Deletion", "Backup deleted!", 1500 );
        } else {
            Notify::show_notification( "Backup Deletion", "Backup could not be deleted!", 1500 );
        }
    }
    ImGui::SetItemTooltip( "Delete backed up savegame" );

    ImGui::PopStyleColor( 2 );
    ImGui::PopStyleVar( );
    ImGui::PopID( );
}

void CBackupsView::render_modals( ) {
    if ( m_open_rename_modal ) {
        m_open_rename_modal = false;
        ImGui::OpenPopup( "Rename Backup" );
    }

    if ( ImGui::BeginPopupModal( "Rename Backup", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) ) {
        ImGui::Text( "%s", m_pending_rename_backup.filename( ).string( ).c_str( ) );
        ImGui::InputText( "Label", &m_rename_input );
        if ( ImGui::Button( "Save" ) ) {
            Features::save_label(
                m_pending_rename_game, m_pending_rename_backup.filename( ).string( ), m_rename_input );
            {
                std::lock_guard lock( m_mutex );
                m_labels_cache[m_pending_rename_game][m_pending_rename_backup.filename( ).string( )] =
                    m_rename_input;
            }
            ImGui::CloseCurrentPopup( );
        }
        ImGui::SameLine( );
        if ( ImGui::Button( "Cancel" ) ) ImGui::CloseCurrentPopup( );
        ImGui::EndPopup( );
    }
}
