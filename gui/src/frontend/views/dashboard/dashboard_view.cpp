#include "dashboard_view.hpp"
#include <utils/utils.hpp>

#include <backend/font_manager/font_manager.hpp>
#include <backend/utils.hpp>

#include <frontend/components/card.hpp>
#include <frontend/components/spinner.hpp>
#include <frontend/icons.hpp>
#include <frontend/notification/notification.hpp>

void CDashboardView::on_enter( ) {
    std::lock_guard<std::mutex> lock( m_result_mutex );
    if ( m_result.empty( ) ) {
        m_last_game_count = 0;
        m_grouped_games.clear( );
        m_game_cache.clear( );
        m_detection_start_time = std::chrono::steady_clock::now( );
        m_refresh_future = std::async( std::launch::async, [this] {
            auto result = Detection::find_saves( m_blacklist, m_translations );
            std::lock_guard lock( m_result_mutex );
            m_result = std::move( result );
        } );
    }
};

void CDashboardView::render( ) {
    m_task_runner.update( );

    bool refresh_done = m_refresh_future.valid( ) &&
                        m_refresh_future.wait_for( std::chrono::seconds( 0 ) ) == std::future_status::ready;

    if ( refresh_done ) {
        m_games_snapshot = m_result;
        m_refresh_future.get( );
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

    if ( m_games_snapshot.size( ) != m_last_game_count ) {
        m_last_game_count = m_games_snapshot.size( );
        on_result_changed( );
    }

    if ( ImGui::BeginTabBar( "##dashboard_tabs" ) ) {
        if ( ImGui::BeginTabItem( "Games" ) ) {
            render_toolbar( );
            render_game_list( );
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
    bool is_refreshing = m_refresh_future.valid( ) &&
                         m_refresh_future.wait_for( std::chrono::seconds( 0 ) ) != std::future_status::ready;
    bool is_backing_up =
        m_backup_future.valid( ) && m_backup_future.wait_for( std::chrono::seconds( 0 ) ) != std::future_status::ready;

    if ( is_refreshing ) {
        Spinner::render( );
    }

    float sort_width = ImGui::CalcTextSize( "Sort: Alphabetical" ).x + ImGui::GetStyle( ).FramePadding.x * 2;
    float filter_width = ImGui::CalcTextSize( "Filter: Rockstar" ).x + ImGui::GetStyle( ).FramePadding.x * 2;
    float refresh_width = ImGui::CalcTextSize( "Refresh" ).x + ImGui::GetStyle( ).FramePadding.x * 2;
    float backup_width = ImGui::CalcTextSize( "Mass Backup" ).x + ImGui::GetStyle( ).FramePadding.x * 2;
    float spacing = ImGui::GetStyle( ).ItemSpacing.x * 3;

    ImGui::TextDisabled(
        "found %zu games in %s seconds", m_filtered_game_count,
        std::format( "{:.2f}s", m_detection_duration ).c_str( ) );

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
    std::string filter_label = m_platform_filter.has_value( )
                                   ? std::format( "{} {}", ICON_FILTER, *m_platform_filter )
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
        m_result.clear( );
        m_last_game_count = 0;
        m_grouped_games.clear( );
        m_game_cache.clear( );
        m_detection_start_time = std::chrono::steady_clock::now( );
        m_refresh_future = std::async( std::launch::async, [this] {
            auto result = Detection::find_saves( m_blacklist, m_translations );
            std::lock_guard lock( m_result_mutex );
            m_result = std::move( result );
        } );
    }
    if ( is_refreshing ) ImGui::EndDisabled( );
    ImGui::SetItemTooltip( "Re-runs the detection logic to find new saves" );
    ImGui::SameLine( );
    if ( is_refreshing || is_backing_up ) ImGui::BeginDisabled( true );
    if ( ImGui::Button( "Mass Backup" ) ) {
        m_pending_invalidate = m_games_snapshot;
        m_backup_future = std::async( std::launch::async, [this]( ) {
            std::vector<Game> snapshot = { };
            {
                std::lock_guard lock( m_result_mutex );
                snapshot = m_result;
            }
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
        m_filtered_game_count++;

        if ( m_platform_filter.has_value( ) && m_games_snapshot[group[0]].platform_label != *m_platform_filter )
            return;

        const Game& primary = m_games_snapshot[group[0]];
        std::string game_name = primary.game_name;

        std::transform( game_name.begin( ), game_name.end( ), game_name.begin( ), ::tolower );
        if ( !m_search_query.empty( ) ) {
            if ( game_name.find( m_search_query ) == std::string::npos ) {
                return;
            }
        }
        render_game_row( group, static_cast<int>( gi ) );
        ImGui::Dummy( ImVec2( 0, 6.0f ) );
    } );
}

void CDashboardView::render_game_content(
    std::pair<int, int> sb_count, const Game& game, bool has_conflicts,
    std::vector<std::pair<fs::path, const Game*>> files ) {

    bool is_refreshing = m_refresh_future.valid( ) &&
                         m_refresh_future.wait_for( std::chrono::seconds( 0 ) ) != std::future_status::ready;
    bool is_backing_up =
        m_backup_future.valid( ) && m_backup_future.wait_for( std::chrono::seconds( 0 ) ) != std::future_status::ready;

    // saves
    if ( sb_count.first <= 0 ) {
        ImGui::TextDisabled( "Game detected but no saves were found!" );
        return;
    }

    if ( game.type != PlatformType::MINECRAFT ) ImGui::TextDisabled( "SAVE FILES" );
    else
        ImGui::TextDisabled( "WORLDS" );

    // TODO: cache this
    fs::path undo_dir = paths::backup_dir( ) / sanitize_filename_path( game.game_name ) / "undo.zip";
    bool has_undo = false;
    if ( fs::exists( undo_dir ) ) has_undo = true;

    // TODO: refactor this, just disable dont hide
    float total = 110.f; // Backup All
    // total += ImGui::CalcTextSize("Create Schedule__").x + 4.f;
    if ( has_conflicts ) total += ImGui::CalcTextSize( "Resolve Conflict(s)__" ).x + 4.f;
    if ( has_undo ) total += ImGui::CalcTextSize( "Undo last restore___" ).x + 4.f;
    ImGui::SameLine( ImGui::GetContentRegionMax( ).x - total );

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
    ImGui::SameLine( );
    if ( is_backing_up || is_refreshing ) ImGui::BeginDisabled( true );
    if ( has_conflicts ) {
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
    }
    if ( is_backing_up || is_refreshing ) ImGui::EndDisabled( );
    ImGui::SameLine( );
    if ( is_backing_up || is_refreshing ) ImGui::BeginDisabled( true );
    if ( has_undo ) {
        if ( ImGui::Button( "Undo last restore" ) ) {
            if ( Features::restore_backup( undo_dir, game.save_paths, m_pending_conflicts ) ) {
                fs::remove( undo_dir );
            } else {
                SPDLOG_ERROR( "Failed to restore backup, kept undo zip!" );
                Notify::show_notification( "Undo Last Restore", "Failed to restore backup, kept undo zip!", 2000 );
            }
            invalidate_cache( { game } );
        }
    }
    if ( is_backing_up || is_refreshing ) ImGui::EndDisabled( );

    for ( auto& save : files ) {
        if ( !fs::exists( save.first ) ) continue;
        if ( save.first.string( ).contains( ".savemgr-conflict-" ) ) continue;

        ImGui::Separator( );
        render_save_row( save.first, *save.second );
        ImGui::Separator( );
    }
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

    for ( const auto& path : cache.save_files ) {
        files.emplace_back( path, &primary );
    }

    auto selectable_id = std::format( "gamename_{}", gi );

    std::string right_text =
        std::format( "{} | {} saves | {} backups", primary.platform_label, save_count, backup_count );
    Card::draw( selectable_id, primary.game_name, not_collapsed, right_text, [&]( ) {
        render_game_content( { save_count, backup_count }, primary, has_conflicts, files );

        if ( backup_count > 0 ) {
            Card::draw( selectable_id, "BACKUPS", bk_collapsed, std::nullopt, [&]( ) {
                for ( auto& backup : backup_paths ) {
                    ImGui::Separator( );
                    render_backup_row( backup, primary, labels );
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

    // if ( not_collapsed )

    // Widgets::end_game_card( );
    // ImGui::EndChild( );
    // ImGui::PopStyleVar( );
}

void CDashboardView::render_save_row( const fs::path& save_file, const Game& game ) {
    ImGui::PushID( save_file.string( ).c_str( ) );

    bool is_backing_up =
        m_backup_future.valid( ) && m_backup_future.wait_for( std::chrono::seconds( 0 ) ) != std::future_status::ready;

    std::string date_text = std::format( "{} | ", format_file_time( fs::last_write_time( save_file ) ) );
    float date_width = ImGui::CalcTextSize( date_text.c_str( ) ).x;

    std::string size_text;
    if ( game.type != PlatformType::MINECRAFT ) {
        auto b_size = fs::file_size( save_file ) / 1024;
        size_text = std::format( "{}KB  ", b_size );
    }

    float size_width = ImGui::CalcTextSize( size_text.c_str( ) ).x;
    float total_width = date_width + size_width + 80.0f * 1 + 4.0f * 5;

    if ( game.show_parent_path ) {
        ImGui::Text(
            "%s", path_to_utf8( save_file.parent_path( ).filename( ) / save_file.filename( ) ).c_str( ) );
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
    if ( ImGui::Button( "Backup", ImVec2( 80.0f, 0 ) ) ) {
        m_pending_invalidate = { game };
        m_backup_future = std::async( std::launch::async, [this, game, save_file, &config = m_config]( ) {
            if ( !Features::backup_game( game, save_file, config ) ) {
                auto str = std::format( "Failed to create backup for: {}", game.game_name );
                Notify::show_notification( "Backup Failure", str, 3000 );
            }
        } );
    }
    if ( is_backing_up ) ImGui::EndDisabled( );
    ImGui::SetItemTooltip( "Create a backup of this save" );
    ImGui::PopStyleVar( );
    ImGui::PopID( );
}

void CDashboardView::render_backup_row(
    const fs::path& backup, const Game& game, const std::unordered_map<std::string, TagCache>& labels ) {

    if ( !fs::exists( backup ) ) {
        SPDLOG_WARN( "backup row skipped, fs::exists() returned false for: {}", path_to_utf8( backup ) );
        return;
    }
    if ( backup.filename( ) == "undo.zip" ) return;

    ImGui::PushID( backup.string( ).c_str( ) );
    std::string backup_filename_utf8 = path_to_utf8( backup.filename( ) );
    auto it = labels.find( backup_filename_utf8 );
    const TagCache* tag_cache = ( it != labels.end( ) ) ? &it->second : nullptr;

    std::string date_text;
    std::string size_text;
    try {
        date_text = std::format( "{} | ", format_file_time( fs::last_write_time( backup ) ) );
        auto b_size = fs::file_size( backup ) / 1024;
        size_text = std::format( "{}KB  ", b_size );
    } catch ( const fs::filesystem_error& ex ) {
        SPDLOG_ERROR( "backup row failed to stat {}: {}", path_to_utf8( backup ), ex.what( ) );
        ImGui::PopID( );
        return;
    }
    float date_width = ImGui::CalcTextSize( date_text.c_str( ) ).x;
    float size_width = ImGui::CalcTextSize( size_text.c_str( ) ).x;

    float total_width = date_width + size_width + 80.0f * 3 + 4.0f * 5;

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
            return;
        }
        m_open_restore_modal = true;
        m_restore_entries = res_entries;
        m_restore_checked.clear( );
        for ( const auto& e : m_restore_entries ) {
            m_restore_checked[e] = true;
        }
    }
    ImGui::CloseCurrentPopup( );

    ImGui::SetItemTooltip( "Restore save from backup" );
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
            auto result =
                Features::save_tags( m_pending_rename_game.game_name, backup_filename_utf8, m_pending_tags );
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
}

void CDashboardView::on_result_changed( ) {
    m_grouped_games = get_grouped( m_games_snapshot );
    m_game_cache.clear( );

    std::set<std::string> labels = { };
    for ( const auto& game : m_games_snapshot )
        if ( !game.platform_label.empty( ) ) labels.insert( game.platform_label );
    m_available_platform_labels.assign( labels.begin( ), labels.end( ) );
    if ( m_platform_filter.has_value( ) && !labels.contains( *m_platform_filter ) ) m_platform_filter = std::nullopt;

    invalidate_cache( m_games_snapshot, [this]( ) {
        m_detection_duration =
            std::chrono::duration<double>( std::chrono::steady_clock::now( ) - m_detection_start_time ).count( );
    } );
}

void CDashboardView::invalidate_cache( const std::vector<Game>& games, std::function<void( )> on_done ) {
    auto temp = std::make_shared<std::unordered_map<std::string, GameCache>>( );
    auto temp_modified = std::make_shared<std::unordered_map<std::string, fs::file_time_type>>( );

    m_task_runner.run(
        [games, temp, temp_modified]( ) {
            for ( const auto& game : games ) {
                GameCache cache;
                for ( const auto& save_path : game.save_paths ) {
                    if ( !fs::is_directory( save_path ) ) continue;
                    if ( save_path.string( ).contains( ".savemgr-conflict-" ) ) continue;

                    if ( game.type != PlatformType::MINECRAFT ) {
                        for ( const auto& file : fs::recursive_directory_iterator(
                                  save_path, fs::directory_options::skip_permission_denied ) ) {

                            if ( !fs::is_regular_file( file ) ) continue;
                            if ( fs::file_size( file ) == 0 ) continue;

                            auto ext = file.path( ).extension( ).string( );
                            if ( game.type != PlatformType::CUSTOM && game.type != PlatformType::GENERIC ) {
                                if ( extension_blocklist.contains( ext ) ) continue;
                            }

                            if ( g_extension_blocklist.contains( ext ) ) continue;
                            cache.save_files.push_back( file.path( ) );
                        }
                    } else {
                        cache.save_files.push_back( save_path );
                    }
                    auto backups = Features::get_backups( game.game_name );
                    cache.backup_count = backups.size( );
                    cache.backup_paths = backups;
                    cache.tags = load_tag_cache( game.game_name );

                    auto key = utils::get_game_identity_key( game ).value;
                    if ( game.type == PlatformType::MINECRAFT ) {
                        if ( ( *temp ).contains( key ) ) {
                            ( *temp )[key].save_files.push_back( save_path );
                        } else {
                            ( *temp )[key] = cache;
                        }
                    } else {
                        ( *temp )[key] = cache;
                    }

                    try {
                        for ( const auto& f : fs::directory_iterator( save_path ) ) {
                            if ( f.path( ).string( ).find( ".savemgr-conflict-" ) != std::string::npos ) {
                                ( *temp )[key].has_conflicts = true;
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
                ( *temp_modified ).insert( { entry.game_name, current_max } );
            }
        },
        [this, temp, temp_modified, on_done]( ) {
            for ( auto& [key, cache] : *temp )
                m_game_cache[key] = std::move( cache );
            for ( auto& [name, t] : *temp_modified )
                m_game_last_modified[name] = t;
            if ( on_done ) on_done( );
        },
        []( const std::exception& ex ) { Notify::show_notification( "Cache Invalidation Error", ex.what( ), 3000 ); } );
}
