#include "backup_view.hpp"
#include <logger.hpp>
#include <utils/paths.hpp>
#include <utils/utils.hpp>

#include <backend/font_manager/font_manager.hpp>

#include <frontend/childguard.hpp>
#include <frontend/components/spinner.hpp>
#include <frontend/notification/notification.hpp>

CBackupsView::~CBackupsView( ) { m_queue.shutdown( ); }

void CBackupsView::on_enter( const std::vector<Game>& games_snapshot ) {
    if ( m_backups.empty( ) || m_reload_backups ) request_refresh( games_snapshot );
}

void CBackupsView::request_refresh( const std::vector<Game>& games_snapshot ) {
    if ( m_refreshing ) return;
    m_reload_backups = false;
    m_refreshing = true;
    m_queue.run<RefreshResult>(
        [games_snapshot]( TaskControl& ) { return scan_backups( games_snapshot ); },
        [this]( RefreshResult result ) {
            m_backups = std::move( result.first );
            m_labels_cache = std::move( result.second );
            m_refreshing = false;
        },
        [this]( const std::exception& ex ) {
            m_refreshing = false;
            Notify::show_notification( "Backup Scan Error", ex.what( ), 3000 );
        } );
}

void CBackupsView::on_exit( ) {}

CBackupsView::RefreshResult CBackupsView::scan_backups( const std::vector<Game>& snapshot ) {
    std::unordered_map<std::string, fs::path> save_path_lookup;

    for ( const auto& game : snapshot ) {
        for ( const auto& save : game.save_paths ) {
            auto name = utils::sanitize_filename( game.game_name );
            save_path_lookup[name] = save;
        }
    }

    std::vector<BackupEntry> backups = { };
    LabelsCache labels_cache;

    for ( const auto& entry : fs::directory_iterator( paths::backup_dir( ) ) ) {
        if ( !entry.is_directory( ) ) continue;

        BackupEntry bentry;
        bentry.name = entry.path( ).filename( );

        std::string name_utf8 = utils::path_to_utf8( bentry.name );
        labels_cache[name_utf8] = Tags::load_tag_cache( name_utf8 );

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

    return { std::move( backups ), std::move( labels_cache ) };
}

void CBackupsView::render( const std::vector<Game>& games_snapshot ) {
    m_queue.update( );

    ChildGuard wrapper(
        "##backup_view", ImVec2( 0, ImGui::GetContentRegionAvail( ).y ), ImGuiChildFlags_None,
        ImGuiWindowFlags_NoBackground );

    if ( m_reload_backups ) request_refresh( games_snapshot );

    if ( m_refreshing ) {
        Spinner::render( );
    } else {
        if ( ImGui::Button( "Refresh" ) ) request_refresh( games_snapshot );
        ImGui::SetItemTooltip( "Rescans the backups directory" );

        ImGui::Dummy( ImVec2( 0, 5.0f ) );

        if ( m_backups.empty( ) ) {
            ImGui::TextDisabled( "No backups have been created yet.." );
        } else {
            for ( const auto& entry : m_backups ) {
                render_game_row( entry, m_labels_cache );
                ImGui::Dummy( ImVec2( 0, 6.0f ) );
            }
        }
    }

    render_modals( );
}

void CBackupsView::render_game_row( const BackupEntry& bentry, const LabelsCache& labels_cache ) {
    std::string name_utf8 = utils::path_to_utf8( bentry.name );
    bool& not_collapsed = m_card_collapsed[name_utf8];

    auto selectable_id = std::format( "##backup_game_{}", name_utf8 );
    std::string right_text = std::format( "{} backups", bentry.entries.size( ) );

    ImGui::PushStyleVar( ImGuiStyleVar_ChildRounding, 4.0f );
    {
        ImGui::PushStyleColor( ImGuiCol_Border, ImVec4( 198 / 255.f, 97 / 255.f, 63 / 255.f, 1.f ) );
        ChildGuard card(
            selectable_id.c_str( ), ImVec2( 0, 0 ), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY );
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
    }
    ImGui::PopStyleVar( );
}

void CBackupsView::render_backup_row(
    fs::path path, const fs::path& save_path, const std::unordered_map<std::string, TagCache>& labels,
    const std::string& game_name ) {
    if ( path.filename( ) == "undo.zip" ) return;
    if ( !fs::exists( path ) ) return;

    ImGui::PushID( path.string( ).c_str( ) );

    auto it = labels.find( utils::path_to_utf8( path.filename( ) ) );
    const TagCache* tag_cache = ( it != labels.end( ) ) ? &it->second : nullptr;

    std::string date_text = "??";
    std::string size_text = "??";
    try {
        date_text = std::format( "{:%d/%m/%y %H:%M} | ", fs::last_write_time( path ) );
        size_text = utils::format_file_size( fs::file_size( path ) );
    } catch ( const fs::filesystem_error& ex ) {
        auto str = std::format( "backup row failed to stat {}: {}", utils::path_to_utf8( path ), ex.what( ) );
        SPDLOG_ERROR( str );
        ImGui::PopID( );
        return;
    }
    float date_width = ImGui::CalcTextSize( date_text.c_str( ) ).x;
    float size_width = ImGui::CalcTextSize( size_text.c_str( ) ).x;
    float spacing = ImGui::GetStyle( ).ItemSpacing.x;
    float total_width = date_width + size_width + 80.0f * 3 + spacing * 5;

    std::string tag_text =
        ( tag_cache && !tag_cache->tags.empty( ) ) ? tag_cache->display : utils::path_to_utf8( path.filename( ) );
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
            Backup::restore_backup( path, { save_path }, conflicts );
        }
    }
    ImGui::SetItemTooltip( "Restore save from backup" );
    ImGui::SameLine( 0.0f, spacing );

    if ( ImGui::Button( "Tags", ImVec2( 80.0f, 0 ) ) ) {
        auto tagz = tag_cache ? tag_cache->tags : std::vector<std::string>{ };
        m_tags_modal.open(
            game_name, path, tagz, [this]( const std::string&, const std::vector<std::string>& ) {
                m_reload_backups = true;
            } );
    }
    ImGui::SetItemTooltip( "Manage tags for this backup" );
    ImGui::SameLine( 0.0f, spacing );

    ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.8f, 0.2f, 0.2f, 1.0f ) );
    ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 0.9f, 0.3f, 0.3f, 1.0f ) );
    if ( ImGui::Button( "Delete", ImVec2( 80.0f, 0 ) ) ) {
        if ( fs::remove( path ) ) {
            Tags::delete_tags( game_name, utils::path_to_utf8( path.filename( ) ) );
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

void CBackupsView::render_modals( ) { m_tags_modal.render( ); }
