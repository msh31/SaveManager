#include "backup_view.hpp"
#include <backend/font_manager/font_manager.hpp>
#include <backend/utils.hpp>
#include <features/features.hpp>
#include <utils/paths.hpp>
#include <utils/utils.hpp>

#include <frontend/components/spinner.hpp>
#include <frontend/notification/notification.hpp>

void CBackupsView::on_enter( const std::vector<Game>& games_snapshot ) {
    if ( m_backups.empty( ) || m_reload_backups ) {
        m_reload_backups = false;
        m_refresh_future =
            std::async( std::launch::async, [this, games_snapshot] { add_new_entry( games_snapshot ); } );
    }
}

void CBackupsView::on_exit( ) {}

void CBackupsView::add_new_entry( std::vector<Game> snapshot ) {
    std::unordered_map<std::string, fs::path> save_path_lookup;

    std::vector<BackupEntry> backups = { };

    std::unordered_map<std::string, std::unordered_map<std::string, TagCache>> labels_cache;

    for ( const auto& game : snapshot ) {
        for ( const auto& save : game.save_paths ) {
            save_path_lookup[sanitize_filename( game.game_name )] = save;
        }
    }

    backups.clear( );

    for ( const auto& entry : fs::directory_iterator( paths::backup_dir( ) ) ) {
        if ( !entry.is_directory( ) ) continue;

        BackupEntry bentry;
        bentry.name = entry.path( ).filename( );

        std::string name_utf8 = path_to_utf8( bentry.name );
        labels_cache[name_utf8] = load_tag_cache( name_utf8 );

        if ( auto it = save_path_lookup.find( name_utf8 ); it != save_path_lookup.end( ) )
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
    m_backups = std::move( backups );
}

void CBackupsView::render( const std::vector<Game>& games_snapshot ) {
    ImGui::BeginChild(
        "##backup_view", ImVec2( 0, ImGui::GetContentRegionAvail( ).y ), false, ImGuiWindowFlags_NoBackground );

    bool is_refreshing = m_refresh_future.valid( ) &&
                         m_refresh_future.wait_for( std::chrono::seconds( 0 ) ) != std::future_status::ready;

    if ( m_refresh_future.valid( ) && !is_refreshing ) m_refresh_future.get( );

    if ( !is_refreshing && m_reload_backups ) {
        m_reload_backups = false;
        m_refresh_future =
            std::async( std::launch::async, [this, games_snapshot] { add_new_entry( games_snapshot ); } );
    }

    std::vector<BackupEntry> snapshot;
    std::unordered_map<std::string, std::unordered_map<std::string, TagCache>> labels_cache;
    {
        std::lock_guard lock( m_mutex );
        snapshot = m_backups;
        labels_cache = m_labels_cache;
    }

    if ( is_refreshing ) {
        Spinner::render( );
    } else {
        if ( ImGui::Button( "Refresh" ) )
            m_refresh_future =
                std::async( std::launch::async, [this, games_snapshot] { add_new_entry( games_snapshot ); } );
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
    const BackupEntry& bentry,
    const std::unordered_map<std::string, std::unordered_map<std::string, TagCache>>& labels_cache ) {
    std::string name_utf8 = path_to_utf8( bentry.name );
    bool& not_collapsed = m_card_collapsed[name_utf8];

    auto selectable_id = std::format( "##backup_game_{}", name_utf8 );
    std::string right_text = std::format( "{} backups", bentry.entries.size( ) );

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
    ImGui::Text( "%s", name_utf8.c_str( ) );
    ImGui::PopFont( );

    ImGui::SameLine( ImGui::GetContentRegionMax( ).x - ImGui::CalcTextSize( right_text.c_str( ) ).x );
    ImGui::Text( "%s", right_text.c_str( ) );

    if ( not_collapsed ) {
        static const std::unordered_map<std::string, TagCache> empty_labels;
        auto it = labels_cache.find( name_utf8 );
        const auto& labels = ( it != labels_cache.end( ) ) ? it->second : empty_labels;
        for ( const auto& entry : bentry.entries )
            render_backup_row( entry, bentry.save_path, labels, name_utf8 );
    }

    ImGui::EndChild( );
    ImGui::PopStyleVar( );
}

void CBackupsView::render_backup_row(
    fs::path path, const fs::path& save_path, const std::unordered_map<std::string, TagCache>& labels,
    const std::string& game_name ) {
    if ( path.filename( ) == "undo.zip" ) return;
    ImGui::PushID( path.string( ).c_str( ) );
    if ( !fs::exists( path ) ) {
        SPDLOG_WARN( "backup row skipped, fs::exists() returned false for: {}", path_to_utf8( path ) );
        ImGui::PopID( );
        return;
    }

    auto it = labels.find( path_to_utf8( path.filename( ) ) );
    const TagCache* tag_cache = ( it != labels.end( ) ) ? &it->second : nullptr;

    std::string date_text = "failed to set";
    std::string size_text = "failed to set";
    try {
        date_text = std::format( "{:%d/%m/%y %H:%M} | ", fs::last_write_time( path ) );
        auto b_size = fs::file_size( path ) / 1024;
        size_text = std::format( "{}KB  ", b_size );
    } catch ( const fs::filesystem_error& ex ) {
        SPDLOG_ERROR( "backup row failed to stat {}: {}", path_to_utf8( path ), ex.what( ) );
        ImGui::PopID( );
        return;
    }
    float date_width = ImGui::CalcTextSize( date_text.c_str( ) ).x;
    float size_width = ImGui::CalcTextSize( size_text.c_str( ) ).x;
    float spacing = ImGui::GetStyle( ).ItemSpacing.x;
    float total_width = date_width + size_width + 80.0f * 3 + spacing * 5;

    std::string tag_text =
        ( tag_cache && !tag_cache->tags.empty( ) ) ? tag_cache->display : path_to_utf8( path.filename( ) );
    ImGui::TextDisabled( "%s", tag_text.c_str( ) );
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
            Features::restore_backup( path, { save_path }, conflicts );
        }
    }
    ImGui::SetItemTooltip( "Restore save from backup" );
    ImGui::SameLine( 0.0f, spacing );

    if ( ImGui::Button( "Tags", ImVec2( 80.0f, 0 ) ) ) {
        m_pending_rename_game = game_name;
        m_pending_rename_backup = path;
        m_pending_tags = tag_cache ? tag_cache->tags : std::vector<std::string>{ };
        m_new_tag_input.clear( );
        m_open_tags_modal = true;
    }
    ImGui::SetItemTooltip( "Manage tags for this backup" );
    ImGui::SameLine( 0.0f, spacing );

    ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.8f, 0.2f, 0.2f, 1.0f ) );
    ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 0.9f, 0.3f, 0.3f, 1.0f ) );
    if ( ImGui::Button( "Delete", ImVec2( 80.0f, 0 ) ) ) {
        if ( fs::remove( path ) ) {
            Features::delete_tags( game_name, path_to_utf8( path.filename( ) ) );
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
    if ( m_open_tags_modal ) {
        m_open_tags_modal = false;
        ImGui::OpenPopup( "Manage Tags" );
    }

    if ( ImGui::BeginPopupModal( "Manage Tags", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) ) {
        ImGui::Text( "%s", path_to_utf8( m_pending_rename_backup.filename( ) ).c_str( ) );
        ImGui::Separator( );

        int remove_index = -1;
        for ( size_t i = 0; i < m_pending_tags.size( ); i++ ) {
            ImGui::PushID( static_cast<int>( i ) );
            ImGui::Text( "%s", m_pending_tags[i].c_str( ) );
            ImGui::SameLine( );
            if ( ImGui::SmallButton( "x" ) ) remove_index = static_cast<int>( i );
            ImGui::PopID( );
        }
        if ( remove_index >= 0 ) m_pending_tags.erase( m_pending_tags.begin( ) + remove_index );

        if ( m_pending_tags.empty( ) ) ImGui::TextDisabled( "No tags yet" );

        ImGui::Separator( );

        bool add_tag = ImGui::InputText( "##new_tag", &m_new_tag_input, ImGuiInputTextFlags_EnterReturnsTrue );
        ImGui::SameLine( );
        add_tag = ImGui::Button( "Add" ) || add_tag;
        if ( add_tag && !m_new_tag_input.empty( ) ) {
            if ( std::ranges::find( m_pending_tags, m_new_tag_input ) == m_pending_tags.end( ) )
                m_pending_tags.push_back( m_new_tag_input );
            m_new_tag_input.clear( );
        }

        ImGui::Dummy( ImVec2( 0, 5.0f ) );
        if ( ImGui::Button( "Save" ) ) {
            std::string backup_filename_utf8 = path_to_utf8( m_pending_rename_backup.filename( ) );
            auto result = Features::save_tags( m_pending_rename_game, backup_filename_utf8, m_pending_tags );
            if ( result.has_value( ) && *result ) {
                std::lock_guard lock( m_mutex );
                auto& game_tags = m_labels_cache[m_pending_rename_game];
                if ( m_pending_tags.empty( ) ) {
                    game_tags.erase( backup_filename_utf8 );
                } else {
                    TagCache tcache;
                    tcache.tags = m_pending_tags;
                    tcache.display = m_pending_tags | std::ranges::views::join_with( std::string_view( ", " ) ) |
                                     std::ranges::to<std::string>( );
                    game_tags[backup_filename_utf8] = std::move( tcache );
                }
            } else {
                Notify::show_notification( "Tags", "Failed to save tags!", 1500 );
            }
            ImGui::CloseCurrentPopup( );
        }
        ImGui::SameLine( );
        if ( ImGui::Button( "Cancel" ) ) ImGui::CloseCurrentPopup( );
        ImGui::EndPopup( );
    }
}
