#include "dashboard_view.hpp"
#include <utils/utils.hpp>

#include <backend/font_manager/font_manager.hpp>
#include <backend/utils.hpp>

#include <frontend/components/card.hpp>
#include <frontend/dialogs/confirm/confirm_dialog.hpp>
#include <frontend/icons.hpp>
#include <frontend/notification/notification.hpp>

void CDashboardView::on_enter( ) { m_detection.ensure_started( ); };

void CDashboardView::render( ) {
    m_task_runner.update( );

    if ( m_detection.generation( ) != m_seen_generation ) {
        m_seen_generation = m_detection.generation( );
        m_games_snapshot = m_detection.snapshot( );
        on_result_changed( );
    }

    bool backup_done =
        m_backup_future.valid( ) && m_backup_future.wait_for( std::chrono::seconds( 0 ) ) == std::future_status::ready;
    if ( backup_done ) {
        m_backup_future.get( );
        if ( !m_pending_invalidate.empty( ) ) {
            invalidate_cache( m_pending_invalidate );
            m_pending_invalidate.clear( );
        }
    }

    if ( ImGui::BeginTabBar( "##dashboard_tabs" ) ) {
        if ( ImGui::BeginTabItem( "Games" ) ) {
            if ( m_detection.is_refreshing( ) ) ImGui::BeginDisabled( true );
            render_toolbar( );
            render_game_list( );
            if ( m_detection.is_refreshing( ) ) ImGui::EndDisabled( );
            ImGui::EndTabItem( );
        }

        bool backups_open = ImGui::BeginTabItem( "Backups" );
        if ( !m_backups_tab_was_active && backups_open ) m_backups_view.on_enter( m_games_snapshot );
        m_backups_tab_was_active = backups_open;
        if ( backups_open ) {
            m_backups_view.render( m_games_snapshot );
            ImGui::EndTabItem( );
        }

        ImGui::EndTabBar( );
    }

    render_modals( );
}

void CDashboardView::on_exit( ) {}
CDashboardView::~CDashboardView( ) { m_task_runner.shutdown( ); }

// private
void CDashboardView::render_toolbar( ) {
    bool is_refreshing = m_detection.is_refreshing( );
    bool is_backing_up =
        m_backup_future.valid( ) && m_backup_future.wait_for( std::chrono::seconds( 0 ) ) != std::future_status::ready;

    float sort_width = ImGui::CalcTextSize( "Sort: Alphabetical" ).x + ImGui::GetStyle( ).FramePadding.x * 2;
    float filter_width = ImGui::CalcTextSize( "Filter: Rockstar" ).x + ImGui::GetStyle( ).FramePadding.x * 2;
    float refresh_width = ImGui::CalcTextSize( "Refresh" ).x + ImGui::GetStyle( ).FramePadding.x * 2;
    float backup_width = ImGui::CalcTextSize( "Mass Backup" ).x + ImGui::GetStyle( ).FramePadding.x * 2;
    float spacing = ImGui::GetStyle( ).ItemSpacing.x * 3;

    ImGui::TextDisabled(
        "found %zu games in %s", m_filtered_game_count,
        std::format( "{:.2f} seconds", m_detection.last_duration( ) ).c_str( ) );

    ImGui::SetNextItemWidth(
        ImGui::GetContentRegionAvail( ).x - sort_width - refresh_width - backup_width - filter_width - spacing );

    if ( ( ImGui::GetIO( ).KeyCtrl && ImGui::IsKeyPressed( ImGuiKey_F ) ) ) {
        m_focus_search = true;
    }
    if ( m_focus_search ) {
        ImGui::SetKeyboardFocusHere( );
        m_focus_search = false;
    }
    if ( ImGui::InputText( "##search", &m_search_query ) ) {
        std::transform( m_search_query.begin( ), m_search_query.end( ), m_search_query.begin( ), ::tolower );
    }
    ImGui::SameLine( );

    std::string sort_label = m_sort_mode == SortMode::Alphabetical ? std::format( "{} A-Z", ICON_SORT )
                                                                   : std::format( "{} Date", ICON_SORT );

    if ( ImGui::Button( sort_label.c_str( ) ) ) {
        m_sort_mode = m_sort_mode == SortMode::Alphabetical ? SortMode::Recent : SortMode::Alphabetical;
    }
    ImGui::SameLine( );
    std::string filter_label = m_platform_filter.has_value( ) ? std::format( "{} {}", ICON_FILTER, *m_platform_filter )
                                                              : std::format( "{} All", ICON_FILTER );
    if ( ImGui::Button( filter_label.c_str( ) ) && !m_available_platform_labels.empty( ) ) {
        if ( !m_platform_filter.has_value( ) ) {
            m_platform_filter = m_available_platform_labels.front( );
        } else {
            auto it = std::ranges::find( m_available_platform_labels, *m_platform_filter );
            if ( it == m_available_platform_labels.end( ) || it + 1 == m_available_platform_labels.end( ) ) {
                m_platform_filter = std::nullopt;
            } else {
                m_platform_filter = *( it + 1 );
            }
        }
    }
    ImGui::SameLine( );

    if ( is_refreshing ) ImGui::BeginDisabled( true );
    bool is_refresh_keybind_pressed = ( ImGui::GetIO( ).KeyCtrl && ImGui::IsKeyPressed( ImGuiKey_R ) );
    if ( ( ImGui::Button( "Refresh" ) || is_refresh_keybind_pressed ) && !is_refreshing ) {
        m_last_game_count = 0;
        m_grouped_games.clear( );
        m_game_cache.clear( );
        m_detection.refresh( );
    }
    if ( is_refreshing ) ImGui::EndDisabled( );
    ImGui::SetItemTooltip( "Re-runs the detection logic to find new saves" );
    ImGui::SameLine( );
    if ( is_refreshing || is_backing_up ) ImGui::BeginDisabled( true );
    if ( ImGui::Button( "Mass Backup" ) ) {
        m_pending_invalidate = m_games_snapshot;
        auto snapshot = m_detection.snapshot( );
        m_backup_future = std::async( std::launch::async, [this, snapshot]( ) {
            if ( snapshot.empty( ) ) {
                Notify::show_notification( "Mass Backup", "Failed to create snapshot of all saves!", 2000 );
                return;
            }

            auto failed_games = Features::backup_all_games( snapshot, m_config );
            if ( !failed_games.empty( ) ) {
                for ( const auto& entry : failed_games ) {
                    auto str = std::format( "Failed to backup {}!", entry );
                    Notify::show_notification( "Mass Backup", str, 1500 );
                }
            } else {
                Notify::show_notification( "Mass Backup", "Succesfully backed up all gamesaves!", 1500 );
            }
        } );
    }
    if ( is_refreshing || is_backing_up ) ImGui::EndDisabled( );
    ImGui::SetItemTooltip( "Creates a backup of all games found!" );

    ImGui::Dummy( ImVec2( 0.0f, 5.0f ) );
}

void CDashboardView::render_game_list( ) {
    m_filtered_game_count = 0;
    auto sorted = m_grouped_games;

    switch ( m_sort_mode ) {
    case SortMode::Recent:
        std::sort( sorted.begin( ), sorted.end( ), [&]( const std::vector<int>& a, const std::vector<int>& b ) {
            return m_game_last_modified[m_games_snapshot[a[0]].game_name] >
                   m_game_last_modified[m_games_snapshot[b[0]].game_name];
        } );
        break;
    case SortMode::Alphabetical:
        std::sort( sorted.begin( ), sorted.end( ), [&]( const std::vector<int>& a, const std::vector<int>& b ) {
            return m_games_snapshot[a[0]].game_name < m_games_snapshot[b[0]].game_name;
        } );
        break;
    }

    enumerate( sorted, [&]( int gi, auto& group ) {
        if ( m_platform_filter.has_value( ) && m_games_snapshot[group[0]].platform_label != *m_platform_filter ) return;
        const Game& primary = m_games_snapshot[group[0]];
        std::string game_name = primary.game_name;

        std::transform( game_name.begin( ), game_name.end( ), game_name.begin( ), ::tolower );
        if ( !m_search_query.empty( ) ) {
            if ( game_name.find( m_search_query ) == std::string::npos ) {
                return;
            }
        }
        m_filtered_game_count++;
        render_game_row( group, static_cast<int>( gi ) );
        ImGui::Dummy( ImVec2( 0, 6.0f ) );
    } );
}

void CDashboardView::render_game_content(
    std::pair<int, int> sb_count, const Game& game, bool has_conflicts,
    std::vector<std::pair<fs::path, const Game*>> files ) {

    auto& cache = m_game_cache[utils::get_game_identity_key( game ).value];
    auto& info = cache.file_info;

    bool is_refreshing = m_detection.is_refreshing( );
    bool is_backing_up =
        m_backup_future.valid( ) && m_backup_future.wait_for( std::chrono::seconds( 0 ) ) != std::future_status::ready;

    // saves
    if ( sb_count.first <= 0 ) {
        ImGui::TextDisabled( "Game detected but no saves were found!" );
        return;
    }

    float total = 110.f; // Backup All
    // total += ImGui::CalcTextSize("Create Schedule__").x + 4.f;
    if ( has_conflicts ) total += ImGui::CalcTextSize( "Resolve Conflict(s)__" ).x + 4.f;
    if ( cache.has_undo ) total += ImGui::CalcTextSize( "Undo last restore___" ).x + 4.f;
    ImGui::SetCursorPosX( ImGui::GetContentRegionMax( ).x - total );

    ImGui::PushStyleColor( ImGuiCol_Button, ImColor( 198, 97, 63 ).Value );
    ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImColor( 198, 97, 63 ).Value );
    if ( is_backing_up || is_refreshing ) ImGui::BeginDisabled( true );
    if ( ImGui::Button( "Backup All" ) ) {
        SPDLOG_INFO( "creating backup of: {}", game.game_name );
        m_pending_invalidate = { game };
        m_backup_future = std::async( std::launch::async, [this, game, files]( ) {
            if ( Features::backup_game_files( game, files ) ) {
                Notify::show_notification( "Backup Created", "A backup has been for all saves!", 1500 );
            } else {
                Notify::show_notification(
                    "Backup Creation", "Failed to create backup! Please refer to the logfile!", 2000 );
            }
        } );
    }
    if ( is_backing_up || is_refreshing ) ImGui::EndDisabled( );
    ImGui::PopStyleColor( 2 );
    if ( has_conflicts ) {
        ImGui::SameLine( );
        if ( is_backing_up || is_refreshing ) ImGui::BeginDisabled( true );
        if ( ImGui::Button( "Resolve Conflict(s)" ) ) {
            m_pending_conflicts.clear( );
            m_pending_conflict_game = game;
            for ( const auto& sp : game.save_paths ) {
                for ( const auto& f : fs::recursive_directory_iterator( sp ) ) {
                    auto full = f.path( ).string( );
                    auto pos = full.find( ".savemgr-conflict-" );
                    if ( pos != std::string::npos ) {
                        fs::path original = full.substr( 0, pos );
                        m_pending_conflicts.push_back( { original, f.path( ) } );
                    }
                }
                m_open_conflict_modal = true;
            }
        }
        if ( is_backing_up || is_refreshing ) ImGui::EndDisabled( );
    }
    if ( cache.has_undo ) {
        ImGui::SameLine( );
        if ( is_backing_up || is_refreshing ) ImGui::BeginDisabled( true );
        if ( ImGui::Button( "Undo last restore" ) ) {
            if ( Features::restore_backup( cache.undo_path, game.save_paths, m_pending_conflicts ) ) {
                fs::remove( cache.undo_path );
            } else {
                SPDLOG_ERROR( "Failed to restore backup, kept undo zip!" );
                Notify::show_notification( "Undo Last Restore", "Failed to restore backup, kept undo zip!", 2000 );
            }
            invalidate_cache( { game } );
        }
        if ( is_backing_up || is_refreshing ) ImGui::EndDisabled( );
    }

    std::string str = "SAVE FILES";
    if ( game.type == PlatformType::MINECRAFT ) str = "WORLDS";

    auto game_key = utils::get_game_identity_key( game ).value;
    if ( !m_saves_expanded.contains( game_key ) ) {
        m_saves_expanded[game_key] = true;
    }
    bool& saves_expanded = m_saves_expanded[game_key];

    auto save_files_id = std::format( "savefiles_{}", game_key );
    Card::draw( save_files_id, str.data( ), saves_expanded, std::nullopt, [&]( ) {
        for ( auto& save : files ) {
            // ugly
            if ( !m_config.d_settings.show_conflicts ) {
                if ( save.first.string( ).contains( ".savemgr-conflict-" ) ) continue;
            }
            render_save_row( save.first, *save.second, info.at( save.first ) );
        }
    } );
}

void CDashboardView::render_game_row( const std::vector<int>& group, int gi ) {
    const Game& primary = m_games_snapshot[group[0]];
    auto game_key = utils::get_game_identity_key( primary ).value;
    if ( !m_backups_expanded.contains( game_key ) ) {
        m_backups_expanded[game_key] = true;
    }

    bool& not_collapsed = m_card_collapsed[game_key];
    bool& bk_collapsed = m_backups_expanded[game_key];
    const char* chevron = bk_collapsed ? "▶" : "▼";

    std::vector<std::pair<fs::path, const Game*>> files = { };

    auto& cache = m_game_cache[game_key];
    int save_count = cache.save_files.size( );
    int backup_count = cache.backup_count;
    auto backup_paths = cache.backup_paths;
    auto labels = cache.tags;
    bool has_conflicts = cache.has_conflicts;
    auto& binfo = cache.backup_info;

    for ( const auto& path : cache.save_files ) {
        files.emplace_back( path, &primary );
    }

    auto selectable_id = std::format( "gamename_{}", game_key );

    std::string right_text =
        std::format( "{} | {} saves | {} backups", primary.platform_label, save_count, backup_count );
    Card::draw( selectable_id, primary.game_name, not_collapsed, right_text, [&]( ) {
        render_game_content( { save_count, backup_count }, primary, has_conflicts, files );

        if ( backup_count > 0 ) {
            Card::draw( selectable_id, "BACKUPS", bk_collapsed, std::nullopt, [&]( ) {
                for ( auto& backup : backup_paths ) {
                    ImGui::Separator( );
                    render_backup_row( backup, primary, labels, binfo.at( backup ) );
                }
            } );
        }
        // }

        if ( ImGui::BeginPopupContextWindow( ) ) {
            if ( ImGui::MenuItem( "Open Path" ) ) {
                // Most games will have one path, front is fine here.
                open_in_file_manager( primary.save_paths.front( ).string( ).c_str( ) );
            }
            // if ( ImGui::BeginMenu( "Schedule backup" ) ) {
            //     for ( const auto &entry : group ) {

            //     }
            //     ImGui::EndMenu( );
            // }
            ImGui::EndPopup( );
        }
    } );
}

void CDashboardView::render_save_row( const fs::path& save_file, const Game& game, const SaveFileInfo& save_info ) {
    if ( !save_info.is_dir && save_info.size <= 0 && m_config.d_settings.skip_empty_files ) {
        return;
    }

    ImGui::PushID( save_file.string( ).c_str( ) );
    ImGui::Separator( );

    bool is_backing_up =
        m_backup_future.valid( ) && m_backup_future.wait_for( std::chrono::seconds( 0 ) ) != std::future_status::ready;

    std::string date_text = std::format( "{} | ", format_file_time( save_info.mtime ) );
    float date_width = ImGui::CalcTextSize( date_text.c_str( ) ).x;

    std::string size_text = "??";
    if ( game.type != PlatformType::MINECRAFT ) { // needs re-thinking
        size_text = std::format( "{}  ", format_file_size( save_info.size ) );
    }

    float size_width = ImGui::CalcTextSize( size_text.c_str( ) ).x;
    auto btn_size = ImVec2( 80.0f, 0 );
    float total_width = date_width + size_width + 80.0f * 3 + 4.0f * 7; // this is fucked up.

    if ( game.show_parent_path ) {
        ImGui::Text( "%s", path_to_utf8( save_file.parent_path( ).filename( ) / save_file.filename( ) ).c_str( ) );
    } else {
        ImGui::Text( "%s", path_to_utf8( save_file.filename( ) ).c_str( ) );
    }

    ImGui::SameLine( ImGui::GetContentRegionMax( ).x - total_width );

    ImGui::TextDisabled( "%s", date_text.c_str( ) );
    ImGui::SameLine( 0.0f, 4.0f );
    ImGui::TextDisabled( "%s", size_text.c_str( ) );
    ImGui::SameLine( 0.0f, 4.0f );

    ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 3.0f, 3.0f ) );
    if ( is_backing_up ) ImGui::BeginDisabled( true );
    if ( ImGui::Button( "Backup", btn_size ) ) {
        m_pending_invalidate = { game };
        m_backup_future = std::async( std::launch::async, [this, game, save_file, &config = m_config]( ) {
            if ( !Features::backup_game( game, save_file, config ) ) {
                auto str = std::format( "Failed to create backup for: {}", game.game_name );
                Notify::show_notification( "Backup Failure", str, 3000 );
            } else {
                auto str = std::format( "Created a backup for: {}!", game.game_name );
                Notify::show_notification( "Backup Creation", str, 3000 );
            }
        } );
    }
    ImGui::SetItemTooltip( "Create a backup of this save" );
    ImGui::SameLine( 0.0f, 4.0f );
    if ( ImGui::Button( "Duplicate", ImVec2( 90.0f, 0 ) ) ) { // also kinda fucked up
        std::error_code ec;

        if ( fs::is_directory( save_file ) ) {
            std::string copy_name = save_file.string( ) + "-savemgr-copy";
            fs::copy( save_file, copy_name, fs::copy_options::recursive, ec );
        } else {
            std::string copy_name = save_file.string( ) + ".savemgr-copy";
            fs::copy_file( save_file, copy_name, ec );
        }

        if ( ec ) {
            SPDLOG_ERROR( "Failed to copy: {} because: {}", path_to_utf8( save_file ), ec.message( ) );
            Notify::show_notification( "Save Duplication", "Save could not be duplicated!", 2500 );
        } else {
            Notify::show_notification( "Save Duplication", "Save duplicated!", 2500 );
            invalidate_cache( { game } );
        }
    }
    ImGui::SameLine( 0.0f, 4.0f );
    ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.8f, 0.2f, 0.2f, 1.0f ) );
    ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 0.9f, 0.3f, 0.3f, 1.0f ) );
    if ( ImGui::Button( "Delete", btn_size ) ) {
        ConfirmDialog::show( "Are you sure?", [this, save_file, game] {
            if ( fs::remove_all( save_file ) ) {
                Notify::show_notification( "Save Deletion", "Save deleted!", 2500 );
                invalidate_cache( { game } );
            } else {
                Notify::show_notification( "Save Deletion", "Save could not be deleted!", 2500 );
            }
        } );
    }
    ImGui::SetItemTooltip( "Delete a savegame" );
    ImGui::PopStyleColor( 2 );
    if ( is_backing_up ) ImGui::EndDisabled( );
    ImGui::PopStyleVar( );
    ImGui::PopID( );
}

void CDashboardView::render_backup_row(
    const fs::path& backup, const Game& game, const std::unordered_map<std::string, TagCache>& labels,
    const SaveFileInfo& info ) {

    // if ( !fs::exists( backup ) ) {
    //     // SPDLOG_WARN( "backup row skipped, fs::exists() returned false for: {}", path_to_utf8( backup ) );
    //     return;
    // }
    // ImGui::PopStyleVar( );
    if ( backup.filename( ) == "undo.zip" ) return;

    ImGui::PushID( backup.string( ).c_str( ) );
    std::string backup_filename_utf8 = path_to_utf8( backup.filename( ) );
    auto it = labels.find( backup_filename_utf8 );
    const TagCache* tag_cache = ( it != labels.end( ) ) ? &it->second : nullptr;

    std::string date_text = "??";
    std::string size_text = "??";
    date_text = std::format( "{} | ", format_file_time( info.mtime ) );
    size_text = format_file_size( info.size );

    float date_width = ImGui::CalcTextSize( date_text.c_str( ) ).x;
    float size_width = ImGui::CalcTextSize( size_text.c_str( ) ).x;

    float total_width = date_width + size_width + 80.0f * 5 + 4.0f * 7; // also fucked like save row

    std::string tag_text = backup_filename_utf8;
    if ( tag_cache && !tag_cache->tags.empty( ) ) {
        tag_text = tag_cache->display;
    }
    ImGui::TextDisabled( "%s", tag_text.c_str( ) );
    ImGui::SameLine( ImGui::GetContentRegionMax( ).x - total_width );

    ImGui::TextDisabled( "%s", date_text.c_str( ) );
    ImGui::SameLine( 0.0f, 4.0f );
    ImGui::TextDisabled( "%s", size_text.c_str( ) );
    ImGui::SameLine( 0.0f, 4.0f );

    ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 3.0f, 3.0f ) );
    if ( ImGui::Button( "Restore", ImVec2( 80.0f, 0 ) ) ) {
        m_game_exclusions_restore = game;
        m_pending_restore_backup = backup;
        auto res_entries = Features::get_backup_entries( backup );
        if ( res_entries.empty( ) ) {
            Notify::show_notification( "Restore Failed", "Found no entries in backup, odd.", 2000 );
            ImGui::PopStyleVar( );
            ImGui::PopID( );
            return;
        }
        m_open_restore_modal = true;
        m_restore_entries = res_entries;
        m_restore_checked.clear( );
        for ( const auto& e : m_restore_entries ) {
            m_restore_checked[e] = true;
        }
    }

    ImGui::SetItemTooltip( "Restore save from backup" );
    ImGui::SameLine( 0.0f, 4.0f );

    if ( ImGui::Button( "Duplicate", ImVec2( 90.0f, 0 ) ) ) { // also kinda fucked up
        std::string bext = backup.extension( ).string( );
        std::string copy_name =
            ( backup.parent_path( ) / ( backup.stem( ).string( ) + ".savemgr-copy" + bext ) ).string( );

        if ( fs::copy_file( backup, copy_name ) ) {
            Notify::show_notification( "Backup Duplication", "Backup duplicated!", 2500 );
            invalidate_cache( { game } );
        } else {
            Notify::show_notification( "Backup Duplication", "Backup could not be duplicated!", 2500 );
        }
    }
    ImGui::SameLine( 0.0f, 4.0f );

    if ( ImGui::Button( "Tags", ImVec2( 80.0f, 0 ) ) ) {
        m_pending_rename_game = game;
        m_pending_rename_backup = backup;
        m_pending_tags = tag_cache ? tag_cache->tags : std::vector<std::string>{ };
        m_new_tag_input.clear( );
        m_open_tags_modal = true;
    }
    ImGui::SetItemTooltip( "Manage tags for this backup" );
    ImGui::SameLine( 0.0f, 4.0f );

    if ( ImGui::Button( "Preview", ImVec2( 80.0f, 0 ) ) ) {
        m_preview_list = Features::get_backup_entries( backup );
        if ( m_preview_list.empty( ) ) {
            // this should not really happen so...
            auto str = std::format( "{} has no files!!", backup.filename( ).string( ) );
            SPDLOG_ERROR( "{} has no entries, that's a bit odd innit", backup.filename( ).string( ) );
            Notify::show_notification( "Preview Failure wtf", str, 2000 );
            return;
        }

        m_open_preview_modal = true;
    }
    ImGui::SetItemTooltip( "Shows the contents of this backup in a pop up dialog" );
    ImGui::SameLine( 0.0f, 4.0f );

    ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.8f, 0.2f, 0.2f, 1.0f ) );
    ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 0.9f, 0.3f, 0.3f, 1.0f ) );
    if ( ImGui::Button( "Delete", ImVec2( 80.0f, 0 ) ) ) {
        if ( fs::remove( backup ) ) {
            if ( Features::delete_tags( game.game_name, backup_filename_utf8 ) ) {
                Notify::show_notification( "Backup Deletion", "Backup deleted!", 1500 );
            } else {
                Notify::show_notification( "Backup Deletion", "Backup could not be deleted!", 1500 );
            }
            invalidate_cache( { game } );
        } else {
            Notify::show_notification( "Backup Deletion", "Backup could not be deleted!", 1500 );
        }
    }
    ImGui::SetItemTooltip( "Delete backed up savegame" );

    ImGui::PopStyleColor( 2 );
    ImGui::PopStyleVar( );
    ImGui::PopID( );
}

// TODO: improve this
// NOTE: its very annoying to manually do this if statement and then the popup itself bla bla bla
void CDashboardView::render_modals( ) {
    if ( m_open_tags_modal ) {
        m_open_tags_modal = false;
        ImGui::OpenPopup( "Manage Tags" );
    }

    if ( m_open_conflict_modal ) {
        m_open_conflict_modal = false;
        ImGui::OpenPopup( "Resolve conflict(s)" );
    }

    if ( m_open_restore_modal ) {
        m_open_restore_modal = false;
        ImGui::OpenPopup( "Restore backup" );
    }

    if ( m_open_preview_modal ) {
        m_open_preview_modal = false;
        ImGui::OpenPopup( "Preview Backup" );
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
            auto result = Features::save_tags( m_pending_rename_game.game_name, backup_filename_utf8, m_pending_tags );
            if ( result.has_value( ) && *result ) {
                auto key = utils::get_game_identity_key( m_pending_rename_game ).value;
                auto& game_tags = m_game_cache[key].tags;
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
        if ( ImGui::Button( "Cancel" ) ) {
            ImGui::CloseCurrentPopup( );
        }
        ImGui::EndPopup( );
    }

    ImGui::SetNextWindowSize( ImVec2( 500, 0 ), ImGuiCond_Always );
    if ( ImGui::BeginPopupModal( "Resolve conflict(s)", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) ) {
        std::vector<int> to_remove;

        for ( size_t i{ }; i < m_pending_conflicts.size( ); i++ ) {
            ImGui::Text( "%s", path_to_utf8( m_pending_conflicts[i].second.filename( ) ).c_str( ) );

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

        if ( !to_remove.empty( ) ) invalidate_cache( { m_pending_conflict_game } );

        if ( m_pending_conflicts.empty( ) ) {
            ImGui::CloseCurrentPopup( );
        }
        if ( ImGui::Button( "Cancel" ) ) {
            ImGui::CloseCurrentPopup( );
        }
        ImGui::EndPopup( );
    }

    if ( ImGui::BeginPopupModal( "Restore backup", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) ) {
        ImGui::TextWrapped(
            "Select all files you would like to restore from %s",
            path_to_utf8( m_pending_restore_backup.filename( ) ).c_str( ) );

        // if ( ImGui::Button( "Select All" ) ) {
        //     m_restore_checked[entry] = !m_restore_checked[entry];
        // }
        auto height = std::min( m_restore_entries.size( ) * ImGui::GetFrameHeightWithSpacing( ) + 1.5f, 400.0f );
        ImGui::BeginChild( "##Restore entries", ImVec2( 650, height ) );
        for ( const auto& entry : m_restore_entries ) {
            ImGui::PushID( entry.c_str( ) );
            std::string text = std::format( "Include '{}'?", path_to_utf8( utf8_to_path( entry ).filename( ) ) );
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

            if ( Features::restore_backup(
                     m_pending_restore_backup, m_game_exclusions_restore.save_paths, m_pending_conflicts,
                     m_pending_exclusions ) ) {
                if ( m_pending_conflicts.empty( ) ) {
                    auto str = std::format(
                        "Successfully restored a backup for: {}",
                        path_to_utf8( m_pending_restore_backup.filename( ) ) );
                    Notify::show_notification( "Backup Restored!", str, 2000 );
                    ImGui::CloseCurrentPopup( );
                } else {
                    ImGui::CloseCurrentPopup( );
                    m_open_conflict_modal = true;
                    Notify::show_notification(
                        "Backup Restore Failed!", "Failed to restore backup due to conflicts, resolve them!", 2000 );
                }
            } else {
                Notify::show_notification( "Restore", "Failed to restore backup!", 2000 );
                ImGui::CloseCurrentPopup( );
            }
            invalidate_cache( { m_game_exclusions_restore } );
        }
        ImGui::SameLine( );
        if ( ImGui::Button( "Cancel" ) ) ImGui::CloseCurrentPopup( );
        ImGui::EndPopup( );
    }

    if ( ImGui::BeginPopupModal( "Preview Backup", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) ) {
        auto height = std::min( m_preview_list.size( ) * ImGui::GetFrameHeightWithSpacing( ) + 1.5f, 400.0f );
        ImGui::BeginChild( "##Preview entries", ImVec2( 650, height ) );
        int i = 0;
        for ( const auto& entry : m_preview_list ) {
            ImGui::PushID( entry.c_str( ) );
            i += 1;
            std::string text = std::format( "{}: {}", i, path_to_utf8( utf8_to_path( entry ).filename( ) ) );
            ImGui::Text( "%s", text.c_str( ) );
            ImGui::Separator( );
            ImGui::PopID( );
        }
        ImGui::EndChild( );

        ImGui::Separator( );
        if ( ImGui::Button( "Ok" ) ) ImGui::CloseCurrentPopup( );
        ImGui::EndPopup( );
    }
}

void CDashboardView::on_result_changed( ) {
    m_grouped_games = get_grouped( m_games_snapshot );
    m_game_cache.clear( );

    std::set<std::string> labels = { };
    for ( const auto& game : m_games_snapshot )
        if ( !game.platform_label.empty( ) ) labels.insert( game.platform_label );
    m_available_platform_labels.assign( labels.begin( ), labels.end( ) );
    if ( m_platform_filter.has_value( ) && !labels.contains( *m_platform_filter ) ) m_platform_filter = std::nullopt;

    invalidate_cache( m_games_snapshot );
}

// TODO: move this out? this is the only user
void CDashboardView::invalidate_cache( const std::vector<Game>& games, std::function<void( )> on_done ) {
    using InvalidateCacheResult =
        std::pair<std::unordered_map<std::string, GameCache>, std::unordered_map<std::string, fs::file_time_type>>;

    bool use_ignore = m_config.d_settings.use_savemgr_ignore;
    m_task_runner.run<InvalidateCacheResult>(
        [games, use_ignore]( ) {
            InvalidateCacheResult result = { };

            for ( const auto& game : games ) {
                GameCache cache;

                auto backups = Features::get_backups( game.game_name );
                cache.backup_count = backups.size( );
                cache.backup_paths = backups;
                for ( const auto& backup : cache.backup_paths ) {
                    auto ftime = fs::last_write_time( backup );
                    auto bsz = fs::file_size( backup );

                    SaveFileInfo sfi = { bsz, ftime, false };
                    cache.backup_info[backup] = sfi;
                }

                cache.tags = load_tag_cache( game.game_name );

                fs::path undo_dir = paths::backup_dir( ) / sanitize_filename_path( game.game_name ) / "undo.zip";
                if ( fs::exists( undo_dir ) ) {
                    cache.undo_path = undo_dir;
                    cache.has_undo = true;
                }

                for ( const auto& save_path : game.save_paths ) {
                    if ( !fs::is_directory( save_path ) ) continue;
                    if ( save_path.string( ).contains( ".savemgr-conflict-" ) ) continue;

                    bool signore_exists = fs::exists( save_path / ".savemgr-ignore" );
                    std::vector<IgnoreRule> ignore_rules = { };
                    if ( use_ignore && signore_exists ) {
                        ignore_rules = Blacklist::parse_ignore_file( save_path / ".savemgr-ignore" );
                    }

                    if ( game.type != PlatformType::MINECRAFT ) {
                        for ( const auto& file : fs::recursive_directory_iterator(
                                  save_path, fs::directory_options::skip_permission_denied ) ) {

                            if ( ignore_rules.empty( ) ) {
                                auto ext = save_path.extension( ).string( );
                                if ( game.type != PlatformType::CUSTOM && game.type != PlatformType::GENERIC ) {
                                    if ( extension_blocklist.contains( ext ) ) continue;
                                }
                                // images, but not svg because old COD games use .svg like BO and Ghosts
                                if ( g_extension_blocklist.contains( ext ) ) continue;
                            } else {
                                if ( Blacklist::is_ignored( fs::relative( file.path( ), save_path ), ignore_rules ) ) {
                                    continue;
                                }
                            }

                            bool is_dir = fs::is_directory( file );
                            if ( is_dir ) continue;

                            uintmax_t fsz = file.file_size( );
                            if ( fsz == 0 ) continue;

                            auto ftime = file.last_write_time( );

                            cache.save_files.push_back( file.path( ) );

                            SaveFileInfo sfi = { fsz, ftime, is_dir };
                            cache.file_info[file.path( )] = sfi;
                        }
                    } else {
                        auto ftime = fs::last_write_time( save_path );
                        SaveFileInfo sfi = { 0, ftime, true };
                        cache.save_files.push_back( save_path );
                        cache.file_info[save_path] = sfi;
                    }

                    auto key = utils::get_game_identity_key( game ).value;
                    if ( game.type == PlatformType::MINECRAFT ) {
                        if ( result.first.contains( key ) ) {
                            auto ftime = fs::last_write_time( save_path );
                            SaveFileInfo sfi = { 0, ftime, true };
                            result.first[key].save_files.push_back( save_path );
                            result.first[key].file_info[save_path] = sfi;
                        } else {
                            result.first[key] = cache;
                        }
                    } else {
                        result.first[key] = cache;
                    }

                    try {
                        for ( const auto& f : fs::directory_iterator( save_path ) ) {
                            if ( f.path( ).string( ).find( ".savemgr-conflict-" ) != std::string::npos ) {
                                result.first[key].has_conflicts = true;
                                break;
                            }
                        }
                    } catch ( std::exception& ex ) {
                        SPDLOG_ERROR( "conflict iteration error: {}", ex.what( ) );
                    }
                }
            }

            for ( const auto& entry : games ) {
                fs::file_time_type current_max;
                for ( const auto& save_path : entry.save_paths ) {
                    if ( !fs::is_directory( save_path ) ) continue;
                    for ( const auto& file :
                          fs::directory_iterator( save_path, fs::directory_options::skip_permission_denied ) ) {
                        if ( !fs::exists( file ) ) continue;
                        auto t = fs::last_write_time( file );
                        if ( fs::is_regular_file( file ) )
                            if ( t > current_max ) current_max = t;
                    }
                }
                result.second.insert( { entry.game_name, current_max } );
            }
            return result;
        },
        [this, on_done]( InvalidateCacheResult result ) {
            for ( auto& [key, cache] : result.first )
                m_game_cache[key] = std::move( cache );
            for ( auto& [name, t] : result.second )
                m_game_last_modified[name] = t;
            if ( on_done ) on_done( );
        },
        []( const std::exception& ex ) {
            SPDLOG_ERROR( "[Cache] Failed to invalidate cache: {}", ex.what( ) );
            Notify::show_notification( "Cache Invalidation Error", ex.what( ), 3000 );
        } );
}
