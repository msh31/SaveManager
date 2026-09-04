#include "home_view.hpp"
#include <utils/utils.hpp>
#include <logger.hpp>

#include <backup/backup.hpp>
#include <tags/tags.hpp>

#include <frontend/components/dialogs/confirm/confirm_dialog.hpp>
#include <frontend/notification/notification.hpp>
#include <frontend/components/card.hpp>
#include <frontend/icons.hpp>

/*
    TODO LIST
    
    1. improve this mechanism because it can go to the wrong path in some cases, to be confirmed
*/

void CHomeView::on_enter( ) {
    
}

void CHomeView::render( ) {
    if ( CDetectionService::get( ).generation( ) != m_seen_generation ) {
        m_seen_generation = CDetectionService::get( ).generation( );
        m_games_snapshot = CDetectionService::get( ).snapshot( );
        //on_result_changed( );
    }

    if ( ImGui::BeginTabBar( "##dashboard_tabs" ) ) {
        if ( ImGui::BeginTabItem( "Games" ) ) {
            bool disable = CDetectionService::get( ).is_refreshing( );
            if ( disable ) ImGui::BeginDisabled( true );
            render_toolbar( );
            render_game_list( );
            if ( disable ) ImGui::EndDisabled( );
            ImGui::EndTabItem( );
        }

        //bool backups_open = ImGui::BeginTabItem( "Backups" );
        //if ( !m_backups_tab_was_active && backups_open ) m_backups_view.on_enter( m_games_snapshot );
        //m_backups_tab_was_active = backups_open;
        //if ( backups_open ) {
        //    m_backups_view.render( m_games_snapshot );
        //    ImGui::EndTabItem( );
        //}

        ImGui::EndTabBar( );
    }

    render_modals( );
}

void CHomeView::on_exit( ) {
    
}

CHomeView::~CHomeView( ) {}

//private
void CHomeView::render_toolbar( ) {
    bool is_refreshing = CDetectionService::get( ).is_refreshing( );
    bool is_backing_up =
        m_backup_future.valid( ) && m_backup_future.wait_for( std::chrono::seconds( 0 ) ) != std::future_status::ready;

    float sort_width = ImGui::CalcTextSize( "Sort: Alphabetical" ).x + ImGui::GetStyle( ).FramePadding.x * 2;
    float filter_width = ImGui::CalcTextSize( "Filter: Rockstar" ).x + ImGui::GetStyle( ).FramePadding.x * 2;
    float refresh_width = ImGui::CalcTextSize( "Refresh" ).x + ImGui::GetStyle( ).FramePadding.x * 2;
    float backup_width = ImGui::CalcTextSize( "Mass Backup" ).x + ImGui::GetStyle( ).FramePadding.x * 2;
    float spacing = ImGui::GetStyle( ).ItemSpacing.x * 3;

    std::string toolbar_text =
        std::format( "found {} games in {:.2f} seconds", m_filtered_game_count, CDetectionService::get( ).last_duration( ) );
    if ( is_refreshing ) {
        auto progress = CDetectionService::get( ).get_detection_progress( );
        toolbar_text = std::format( "{} of {} complete", progress.first, progress.second );
    }

    ImGui::TextDisabled( "%s", toolbar_text.c_str( ) );
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
        CDetectionService::get( ).refresh( );
    }
    if ( is_refreshing ) ImGui::EndDisabled( );
    ImGui::SetItemTooltip( "Re-runs the detection logic to find new saves" );
    ImGui::SameLine( );
    if ( is_refreshing || is_backing_up ) ImGui::BeginDisabled( true );
    if ( ImGui::Button( "Mass Backup" ) ) {
        m_pending_invalidate = m_games_snapshot;
        auto snapshot = CDetectionService::get( ).snapshot( );
        m_backup_future = std::async( std::launch::async, [this, snapshot]( ) {
            if ( snapshot.empty( ) ) {
                Notify::show_notification( "Mass Backup", "Failed to create snapshot of all saves!", 2000 );
                return;
            }

            auto failed_games = Backup::backup_all_games( snapshot );
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

void CHomeView::render_game_list() {
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

    utils::enumerate( sorted, [&]( int gi, auto& group ) {
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

void CHomeView::render_game_content(
    std::pair<int, int> sb_count, const Game& game, bool has_conflicts,
    std::vector<std::pair<fs::path, const Game*>> files) {

    if ( sb_count.first <= 0 ) {
        ImGui::TextDisabled( "Game detected but no saves were found!" );
        return;
    }

    auto& cache = m_game_cache[utils::get_game_identity_key( game ).value];
    auto& info = cache.file_info;

    bool is_refreshing = CDetectionService::get( ).is_refreshing( );
    bool is_backing_up =
        m_backup_future.valid( ) && m_backup_future.wait_for( std::chrono::seconds( 0 ) ) != std::future_status::ready;

    float total = 110.f; // base sizew of Backup All
    total += ImGui::CalcTextSize( "Create filter__" ).x + 4.f;
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
            if ( Backup::backup_game_files( game, files ) ) {
                Notify::show_notification( "Backup Created", "A backup has been for all saves!", 1500 );
            } else {
                Notify::show_notification(
                    "Backup Creation", "Failed to create backup! Please refer to the logfile!", 2000 );
            }
        } );
    }
    if ( is_backing_up || is_refreshing ) ImGui::EndDisabled( );
    ImGui::PopStyleColor( 2 );

    ImGui::SameLine( );

    if ( is_backing_up || is_refreshing ) ImGui::BeginDisabled( true );
    if ( ImGui::Button( "Create Ruleset" ) ) {
        m_ruleset_modal.open( game, []( const Game& g ) {} );
    }
    if ( is_backing_up || is_refreshing ) ImGui::EndDisabled( );

    if ( has_conflicts ) {
        ImGui::SameLine( );
        if ( is_backing_up || is_refreshing ) ImGui::BeginDisabled( true );
        if ( ImGui::Button( "Conflicts" ) ) {
            std::vector<std::pair<fs::path, fs::path>> conflicts = { };

            for ( const auto& sp : game.save_paths ) {
                for ( const auto& f : fs::recursive_directory_iterator( sp ) ) {
                    auto full = f.path( ).string( );
                    auto pos = full.find( ".savemgr-conflict-" );
                    if ( pos != std::string::npos ) {
                        fs::path original = full.substr( 0, pos );
                        conflicts.push_back( { original, f.path( ) } );
                    }
                }
            }

            m_conflicts_modal.open( game, conflicts, []( const Game& g ) {} );
        }
        if ( is_backing_up || is_refreshing ) ImGui::EndDisabled( );
    }

    if ( cache.has_undo ) {
        ImGui::SameLine( );
        if ( is_backing_up || is_refreshing ) ImGui::BeginDisabled( true );
        //if ( ImGui::Button( "Undo last restore" ) ) {
        //    if ( Backup::restore_backup( cache.undo_path, game.save_paths, m_pending_conflicts ) ) {
        //        fs::remove( cache.undo_path );
        //    } else {
        //        SPDLOG_ERROR( "Failed to restore backup, kept undo zip!" );
        //        Notify::show_notification( "Undo Last Restore", "Failed to restore backup, kept undo zip!", 2000 );
        //    }
        //    //invalidate_cache( { game } );
        //}
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
            //if ( !CConfig::get().d_settings.show_conflicts ) {
            //    if ( save.first.string( ).contains( ".savemgr-conflict-" ) ) continue;
            //}
            render_save_row( save.first, *save.second, info.at( save.first ) );
        }
    } );
}

void CHomeView::render_game_row( const std::vector<int>& group, int gi ) {
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

    std::string prt = { };
#ifndef NDEBUG
    prt = std::format( "{} | ", primary.platform_label );
#endif
    std::string right_text = std::format( "{}{} saves | {} backups", prt, save_count, backup_count );

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

        //1. 
        if ( ImGui::BeginPopupContextWindow( ) ) {
            if ( ImGui::MenuItem( "Open Path" ) ) {
                // Most games will have one path, front is fine here.
                utils::open_in_file_manager( primary.save_paths.front( ).string( ).c_str( ) );
            }
            ImGui::EndPopup( );
        }
    } );
}

void CHomeView::render_backup_row(
    const fs::path& backup, const Game& game, const std::unordered_map<std::string, TagCache>& labels,
    const SaveFileInfo& info ) {

    if ( backup.filename( ) == "undo.zip" ) return;

    ImGui::PushID( backup.string( ).c_str( ) );
    std::string backup_filename_utf8 = utils::path_to_utf8( backup.filename( ) );
    auto it = labels.find( backup_filename_utf8 );
    const TagCache* tag_cache = ( it != labels.end( ) ) ? &it->second : nullptr;

    std::string date_text = "??";
    std::string size_text = "??";
    date_text = std::format( "{} | ", utils::format_file_time( info.mtime ) );
    size_text = utils::format_file_size( info.size );

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
        m_restore_modal.open(
            game, backup, []( const Game& game ) {},
            [this]( const Game& game, const std::vector<std::pair<fs::path, fs::path>>& conflicts ) {
                m_conflicts_modal.open( game, conflicts, []( const Game& g ) {} );
            } );
    }

    ImGui::SetItemTooltip( "Restore save from backup" );
    ImGui::SameLine( 0.0f, 4.0f );

    if ( ImGui::Button( "Duplicate", ImVec2( 90.0f, 0 ) ) ) { // also kinda fucked up
        std::string bext = backup.extension( ).string( );
        std::string copy_name =
            ( backup.parent_path( ) / ( backup.stem( ).string( ) + ".savemgr-copy" + bext ) ).string( );

        if ( fs::copy_file( backup, copy_name ) ) {
            Notify::show_notification( "Backup Duplication", "Backup duplicated!", 2500 );
            //invalidate_cache( { game } );
        } else {
            Notify::show_notification( "Backup Duplication", "Backup could not be duplicated!", 2500 );
        }
    }

    ImGui::SameLine( 0.0f, 4.0f );
    if ( ImGui::Button( "Tags", ImVec2( 80.0f, 0 ) ) ) {
        auto tagz = tag_cache ? tag_cache->tags : std::vector<std::string>{ };
        m_tags_modal.open( game, backup, tagz, []( const std::string& filename, const std::vector<std::string>& tags ) {
            Notify::show_notification( "Tags", "Added tags!", 2000 );
        } );
    }
    ImGui::SetItemTooltip( "Manage tags for this backup" );

    ImGui::SameLine( 0.0f, 4.0f );
    if ( ImGui::Button( "Preview", ImVec2( 80.0f, 0 ) ) ) {
        auto list = Backup::get_backup_entries( backup );
        if ( list.empty( ) ) {
            // this should not really happen so...
            auto str = std::format( "{} has no files!!", backup.filename( ).string( ) );
            SPDLOG_ERROR( "{} has no entries, that's a bit odd innit", backup.filename( ).string( ) );
            Notify::show_notification( "Preview Failure wtf", str, 2000 );
            return;
        }

        m_preview_modal.open( list );
    }
    ImGui::SetItemTooltip( "Shows the contents of this backup in a pop up dialog" );
    ImGui::SameLine( 0.0f, 4.0f );

    ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.8f, 0.2f, 0.2f, 1.0f ) );
    ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 0.9f, 0.3f, 0.3f, 1.0f ) );
    if ( ImGui::Button( "Delete", ImVec2( 80.0f, 0 ) ) ) {
        if ( fs::remove( backup ) ) {
            if ( Tags::delete_tags( game.game_name, backup_filename_utf8 ) ) {
                Notify::show_notification( "Backup Deletion", "Backup deleted!", 1500 );
            } else {
                Notify::show_notification( "Backup Deletion", "Backup could not be deleted!", 1500 );
            }
            //invalidate_cache( { game } );
        } else {
            Notify::show_notification( "Backup Deletion", "Backup could not be deleted!", 1500 );
        }
    }
    ImGui::SetItemTooltip( "Delete backed up savegame" );

    ImGui::PopStyleColor( 2 );
    ImGui::PopStyleVar( );
    ImGui::PopID( );
}

void CHomeView::render_save_row( const fs::path& save_file, const Game& game, const SaveFileInfo& save_info ) {
    //if ( !save_info.is_dir && save_info.size <= 0 && CConfig::get().d_settings.skip_empty_files ) {
    //    return;
    //}
    if ( save_file.filename( ).string( ) == ".savemgr-ignore" ) return;

    ImGui::PushID( save_file.string( ).c_str( ) );
    ImGui::Separator( );

    bool is_backing_up =
        m_backup_future.valid( ) && m_backup_future.wait_for( std::chrono::seconds( 0 ) ) != std::future_status::ready;

    std::string date_text = std::format( "{} | ", utils::format_file_time( save_info.mtime ) );
    float date_width = ImGui::CalcTextSize( date_text.c_str( ) ).x;

    std::string size_text = "??";
    if ( game.type != PlatformType::MINECRAFT ) { // needs re-thinking
        size_text = std::format( "{}  ", utils::format_file_size( save_info.size ) );
    }

    float size_width = ImGui::CalcTextSize( size_text.c_str( ) ).x;
    auto btn_size = ImVec2( 80.0f, 0 );
    float total_width = date_width + size_width + 80.0f * 3 + 4.0f * 7; // this is fucked up.

    if ( game.show_parent_path ) {
        ImGui::Text( "%s", utils::path_to_utf8( save_file.parent_path( ).filename( ) / save_file.filename( ) ).c_str( ) );
    } else {
        ImGui::Text( "%s", utils::path_to_utf8( save_file.filename( ) ).c_str( ) );
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
        m_backup_future = std::async( std::launch::async, [this, game, save_file, &config = CConfig::get()]( ) {
            if ( !Backup::backup_game( game, save_file ) ) {
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
            SPDLOG_ERROR( "Failed to copy: {} because: {}", utils::path_to_utf8( save_file ), ec.message( ) );
            Notify::show_notification( "Save Duplication", "Save could not be duplicated!", 2500 );
        } else {
            Notify::show_notification( "Save Duplication", "Save duplicated!", 2500 );
            //invalidate_cache( { game } );
        }
    }
    ImGui::SameLine( 0.0f, 4.0f );
    ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.8f, 0.2f, 0.2f, 1.0f ) );
    ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 0.9f, 0.3f, 0.3f, 1.0f ) );
    if ( ImGui::Button( "Delete", btn_size ) ) {
        ConfirmDialog::show( "Are you sure?", [this, save_file, game] {
            if ( fs::remove_all( save_file ) ) {
                Notify::show_notification( "Save Deletion", "Save deleted!", 2500 );
                //invalidate_cache( { game } );
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

void CHomeView::render_modals() {
    m_tags_modal.render( );
    m_conflicts_modal.render( );
    m_preview_modal.render( );
    m_ruleset_modal.render( );
    m_restore_modal.render( );
}