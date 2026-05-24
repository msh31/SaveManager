#include "dashboard_view.hpp"
#include <backend/utils.hpp>
#include <utils/utils.hpp>

#include <backend/font_manager/font_manager.hpp>
#include <frontend/components/spinner.hpp>
#include <frontend/notification/notification.hpp>

#include <features/backup/backup.hpp>

// TODO: move this to a dedicated icon header
static constexpr const char* ICON_SORT   = "\xef\x83\x9c";
static constexpr const char* ICON_FILTER = "\xef\x82\xb0";

void CDashboardView::on_enter( ) {
    if ( m_result.games.empty( ) ) {
        {
            std::unique_lock lock( m_result.d_mutex );
            m_result.games.clear( );
        }
        m_last_game_count = 0;
        m_grouped_games.clear( );
        m_game_cache.clear( );
        m_refresh_future = std::async( std::launch::async, [this] { Detection::find_saves( m_config, m_result ); } );
    }
};

void CDashboardView::render( ) {
    bool needs_update = false;
    {
        std::shared_lock lock( m_result.d_mutex );
        size_t           current_count = m_result.games.size( );
        if ( current_count != m_last_game_count ) {
            m_last_game_count = current_count;
            needs_update      = true;
        }
    }
    if ( needs_update ) on_result_changed( );

    render_toolbar( );
    render_game_list( );
    render_modals( );
}

void CDashboardView::on_exit( ) {}
CDashboardView::~CDashboardView( ) {}

// private
void CDashboardView::render_toolbar( ) {
    bool is_refreshing = m_refresh_future.valid( ) &&
                         m_refresh_future.wait_for( std::chrono::seconds( 0 ) ) != std::future_status::ready;
    bool is_backing_up =
        m_backup_future.valid( ) && m_backup_future.wait_for( std::chrono::seconds( 0 ) ) != std::future_status::ready;

    float sort_width    = ImGui::CalcTextSize( "Sort: Alphabetical" ).x + ImGui::GetStyle( ).FramePadding.x * 2;
    float filter_width  = ImGui::CalcTextSize( "Filter: Rockstar" ).x + ImGui::GetStyle( ).FramePadding.x * 2;
    float refresh_width = ImGui::CalcTextSize( "Refresh" ).x + ImGui::GetStyle( ).FramePadding.x * 2;
    float backup_width  = ImGui::CalcTextSize( "Mass Backup" ).x + ImGui::GetStyle( ).FramePadding.x * 2;
    float spacing       = ImGui::GetStyle( ).ItemSpacing.x * 3;

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
                                   ? std::format( "{} {}", ICON_FILTER, get_platform_label( *m_platform_filter ) )
                                   : std::format( "{} All", ICON_FILTER );
    if ( ImGui::Button( filter_label.c_str( ) ) ) {
        if ( !m_platform_filter.has_value( ) ) {
            m_platform_filter = PlatformType::UBISOFT;
        } else {
            switch ( *m_platform_filter ) {
            case PlatformType::UBISOFT:
                m_platform_filter = PlatformType::ROCKSTAR;
                break;
            case PlatformType::ROCKSTAR:
                m_platform_filter = PlatformType::UNREAL;
                break;
            case PlatformType::UNREAL:
                m_platform_filter = PlatformType::CUSTOM;
                break;
            case PlatformType::CUSTOM:
                m_platform_filter = std::nullopt;
                break;
            default:
                m_platform_filter = std::nullopt;
                break;
            }
        }
    }
    ImGui::SameLine( );

    if ( !is_refreshing &&
         ( ImGui::Button( "Refresh" ) || ( ImGui::GetIO( ).KeyCtrl && ImGui::IsKeyPressed( ImGuiKey_R ) ) ) ) {
        {
            std::unique_lock lock( m_result.d_mutex );
            m_result.games.clear( );
        }
        m_last_game_count = 0;
        m_grouped_games.clear( );
        m_game_cache.clear( );
        m_refresh_future = std::async( std::launch::async, [this] { Detection::find_saves( m_config, m_result ); } );
    }
    ImGui::SetItemTooltip( "Re-runs the detection logic to find new saves" );
    ImGui::SameLine( );
    if ( !is_backing_up && !is_refreshing ) {
        if ( ImGui::Button( "Mass Backup" ) ) {
            m_backup_future = std::async( std::launch::async, [&result = m_result, &config = m_config]( ) {
                std::vector<Game> snapshot;
                {
                    std::shared_lock lock( result.d_mutex );
                    snapshot = result.games;
                }
                Features::backup_all_games( snapshot, config );
                Notify::show_notification( "Mass Backup", "Succesfully backed up all gamesaves!", 1500 );
            } );
        }
        ImGui::SetItemTooltip( "Creates a backup of all games found!" );
    } else {
        Spinner::render( );
    }

    ImGui::Dummy( ImVec2( 0.0f, 5.0f ) );
}

void CDashboardView::render_game_list( ) {
    auto sorted = m_grouped_games;
    switch ( m_sort_mode ) {
    case SortMode::Recent:
        std::sort( sorted.begin( ), sorted.end( ), [&]( const std::vector<int>& a, const std::vector<int>& b ) {
            return m_game_last_modified[m_result.games[a[0]].game_name] >
                   m_game_last_modified[m_result.games[b[0]].game_name];
        } );
        break;
    case SortMode::Alphabetical:
        std::sort( sorted.begin( ), sorted.end( ), [&]( const std::vector<int>& a, const std::vector<int>& b ) {
            return m_result.games[a[0]].game_name < m_result.games[b[0]].game_name;
        } );
        break;
    }

    enumerate( sorted, [&]( int gi, auto& group ) {
        if ( m_platform_filter.has_value( ) && m_result.games[group[0]].type != *m_platform_filter ) return;

        const Game& primary   = m_result.games[group[0]];
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

void CDashboardView::render_game_row( const std::vector<int>& group, int gi ) {
    const Game& primary = m_result.games[group[0]];

    bool&       not_collapsed = m_card_collapsed[cache_key( primary )];
    bool&       bk_collapsed  = m_backups_collapsed[cache_key( primary )];
    const char* chevron       = bk_collapsed ? "▶" : "▼";

    std::vector<std::pair<fs::path, const Game*>> files = { };

    auto& cache         = m_game_cache[cache_key( primary )];
    int   save_count    = cache.save_files.size( );
    int   backup_count  = cache.backup_count;
    auto  labels        = cache.labels;
    bool  has_conflicts = cache.has_conflicts;

    for ( const auto& path : cache.save_files ) {
        files.emplace_back( path, &primary );
    }

    auto selectable_id = std::format( "##gamename_{}", gi );

    std::string right_text =
        std::format( "{} | {} saves | {} backups", get_platform_label( primary.type ), save_count, backup_count );

    ImGui::PushStyleColor( ImGuiCol_Border, ImVec4( 198 / 255.f, 97 / 255.f, 63 / 255.f, 1.f ) );
    ImGui::PushStyleVar( ImGuiStyleVar_ChildRounding, 4.0f );
    ImGui::BeginChild( selectable_id.c_str( ), ImVec2( 0, 0 ), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY );
    ImGui::PopStyleColor( );

    if ( ImGui::Selectable( "##header", false, ImGuiSelectableFlags_None, ImVec2( 0, 30 ) ) )
        not_collapsed = !not_collapsed;
    ImGui::SameLine( 8.0f );

    ImGui::PushFont( CFontManager::get( ).get_font( "jbm_bold" ).value_or( nullptr ) );
    ImGui::TextColored( ImColor( 198, 97, 63 ).Value, "%s", chevron );
    ImGui::PopFont( );
    ImGui::SameLine( );

    ImGui::PushFont( CFontManager::get( ).get_font( "jbm_med" ).value_or( nullptr ) );
    ImGui::Text( "%s", primary.game_name.c_str( ) );
    ImGui::PopFont( );

    ImGui::SameLine( ImGui::GetContentRegionMax( ).x - ImGui::CalcTextSize( right_text.c_str( ) ).x );
    ImGui::Text( "%s", right_text.c_str( ) );

    if ( not_collapsed ) {
        if ( save_count > 0 ) {
            if ( primary.type != PlatformType::MINECRAFT ) ImGui::TextDisabled( "SAVE FILES" );
            else
                ImGui::TextDisabled( "WORLDS" );

            // todo cache this
            fs::path undo_dir = paths::backup_dir( ) / sanitize_filename( primary.game_name ) / "undo.zip";
            bool     has_undo = false;
            if ( fs::exists( undo_dir ) ) has_undo = true;

            float total = 110.f; // Backup All
            // total += ImGui::CalcTextSize("Create Schedule__").x + 4.f;
            if ( has_conflicts ) total += ImGui::CalcTextSize( "Resolve Conflict(s)__" ).x + 4.f;
            if ( has_undo ) total += ImGui::CalcTextSize( "Undo last restore___" ).x + 4.f;
            ImGui::SameLine( ImGui::GetContentRegionMax( ).x - total );

            ImGui::PushStyleColor( ImGuiCol_Button, ImColor( 198, 97, 63 ).Value );
            ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImColor( 198, 97, 63 ).Value );
            if ( ImGui::Button( "Backup All" ) ) {
                SPDLOG_INFO( "creating backup of: {}", primary.game_name );
                m_backup_future = std::async( std::launch::async, [this, primary, files]( ) {
                    if ( Features::backup_game_files( primary, files ) ) {
                        Notify::show_notification( "Backup Created", "A backup has been for all saves!", 1500 );
                    } else {
                        Notify::show_notification(
                            "Backup Creation", "Failed to create backup! Please refer to the logfile!", 2000 );
                    }
                } );
            }
            ImGui::PopStyleColor( 2 );
            // #ifndef NDEBUG
            //             ImGui::SameLine( );
            //             if ( ImGui::Button( "Create Schedule" ) ) {
            //                 scheduled_files.clear( );
            //                 scheduled_files_selected.clear( );
            //
            //                 pending_schedule_game = primary;
            //                 open_schedule_modal = true;
            //                 for ( const auto &entry : game_cache[cache_key( primary )].save_files ) {
            //                     scheduled_files.push_back( entry );
            //                 }
            //             }
            // #endif
            ImGui::SameLine( );
            if ( has_conflicts ) {
                if ( ImGui::Button( "Resolve Conflict(s)" ) ) {
                    m_pending_conflicts.clear( );
                    for ( const auto& f : fs::directory_iterator( primary.save_path ) ) {
                        auto     full     = f.path( ).string( );
                        auto     pos      = full.find( ".savemgr-conflict-" );
                        fs::path original = full.substr( 0, pos );
                        m_pending_conflicts.push_back( { original, f.path( ) } );
                    }
                    m_open_conflict_modal = true;
                }
            }
            ImGui::SameLine( );
            if ( has_undo ) {
                if ( ImGui::Button( "Undo last restore" ) ) {
                    Features::restore_backup( undo_dir, primary.save_path, m_pending_conflicts );
                    fs::remove( undo_dir );
                }
            }

            for ( auto& save : files ) {
                if ( !fs::exists( save.first ) ) continue;
                if ( save.first.string( ).contains( ".savemgr-conflict-" ) ) continue;

                ImGui::Separator( );
                render_save_row( save.first, *save.second );
                ImGui::Separator( );
            }
        } else
            ImGui::TextDisabled( "Game detected but no saves were found!" );
        if ( backup_count > 0 ) {
            if ( ImGui::Selectable( "##backups", false, ImGuiSelectableFlags_None, ImVec2( 0, 30 ) ) ) {
                bk_collapsed = !bk_collapsed;
            }
            ImGui::SameLine( 8.0f );

            ImGui::PushFont( CFontManager::get( ).get_font( "jbm_bold" ).value_or( nullptr ) );
            ImGui::TextColored( ImColor( 198, 97, 63 ).Value, "%s", chevron );
            ImGui::PopFont( );
            ImGui::SameLine( );
            ImGui::PushFont( CFontManager::get( ).get_font( "jbm_med" ).value_or( nullptr ) );
            ImGui::Text( "BACKUPS" );
            ImGui::PopFont( );

            if ( !bk_collapsed ) {
                auto backups = Features::get_backups( primary.game_name, m_config );
                for ( auto& backup : backups ) {
                    ImGui::Separator( );
                    render_backup_row( backup, primary, labels );
                }
            }
        }
    }

    if ( ImGui::BeginPopupContextWindow( ) ) {
        if ( ImGui::MenuItem( "Open Path" ) ) {
            open_in_file_manager( primary.save_path.string( ).c_str( ) );
        }
        // if ( ImGui::BeginMenu( "Schedule backup" ) ) {
        //     for ( const auto &entry : group ) {
        //         const Game &g = ctx.games[entry];
        //         std::string label = std::format( "{}##entry_{}", g.save_path.filename( ).string( ), entry );
        //         if ( ImGui::MenuItem( label.c_str( ) ) ) {
        //             pending_schedule_game = g;
        //             open_schedule_modal = true;
        //             ImGui::OpenPopup( "Add schedule" );
        //         }
        //     }
        //     ImGui::EndMenu( );
        // }
        ImGui::EndPopup( );
    }

    // Widgets::end_game_card( );
    ImGui::EndChild( );
    ImGui::PopStyleVar( );
}

void CDashboardView::render_save_row( const fs::path& save_file, const Game& game ) {
    ImGui::PushID( save_file.string( ).c_str( ) );

    std::string date_text  = std::format( "{:%d/%m/%y %H:%M} | ", fs::last_write_time( save_file ) );
    float       date_width = ImGui::CalcTextSize( date_text.c_str( ) ).x;

    std::string size_text;
    if ( game.type != PlatformType::MINECRAFT ) {
        auto b_size = fs::file_size( save_file ) / 1024;
        size_text   = std::format( "{}KB  ", b_size );
    }

    float size_width  = ImGui::CalcTextSize( size_text.c_str( ) ).x;
    float total_width = date_width + size_width + 80.0f * 1 + 4.0f * 5;

    if ( game.show_parent_path ) {
        ImGui::Text( "%s", ( save_file.parent_path( ).filename( ) / save_file.filename( ) ).string( ).c_str( ) );
    } else {
        ImGui::Text( "%s", save_file.filename( ).string( ).c_str( ) );
    }

    ImGui::SameLine( ImGui::GetContentRegionMax( ).x - total_width );

    ImGui::TextDisabled( "%s", date_text.c_str( ) );
    ImGui::SameLine( 0.0f, 4.0f );
    ImGui::TextDisabled( "%s", size_text.c_str( ) );
    ImGui::SameLine( 0.0f, 4.0f );

    ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 3.0f, 3.0f ) );
    if ( ImGui::Button( "Backup", ImVec2( 80.0f, 0 ) ) ) {
        m_backup_future = std::async( std::launch::async, [this, game, save_file, &config = m_config]( ) {
            Features::backup_game( game, save_file, config );
        } );
    }
    ImGui::SetItemTooltip( "Create a backup of this save" );
    ImGui::PopStyleVar( );
    ImGui::PopID( );
}

void CDashboardView::render_backup_row(
    const fs::path& backup, const Game& game, const std::unordered_map<std::string, std::string>& labels ) {

    if ( backup.filename( ) == "undo.zip" ) return;
    ImGui::PushID( backup.string( ).c_str( ) );
    if ( !fs::exists( backup ) ) {
        ImGui::PopID( );
        return;
    }

    auto        it      = labels.find( backup.filename( ).string( ) );
    std::string display = ( it != labels.end( ) ) ? it->second : backup.filename( ).string( );

    std::string date_text  = std::format( "{:%d/%m/%y %H:%M} | ", fs::last_write_time( backup ) );
    float       date_width = ImGui::CalcTextSize( date_text.c_str( ) ).x;
    auto        b_size     = fs::file_size( backup ) / 1024;

    std::string size_text  = std::format( "{}KB  ", b_size );
    float       size_width = ImGui::CalcTextSize( size_text.c_str( ) ).x;

    float total_width = date_width + size_width + 80.0f * 3 + 4.0f * 5;

    ImGui::Text( "%s", display.c_str( ) );
    ImGui::SameLine( ImGui::GetContentRegionMax( ).x - total_width );

    ImGui::TextDisabled( "%s", date_text.c_str( ) );
    ImGui::SameLine( 0.0f, 4.0f );
    ImGui::TextDisabled( "%s", size_text.c_str( ) );
    ImGui::SameLine( 0.0f, 4.0f );

    ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 3.0f, 3.0f ) );
    if ( ImGui::Button( "Restore", ImVec2( 80.0f, 0 ) ) ) {
        if ( game.save_path.empty( ) ) {
            Notify::show_notification( "Restore", "Cannot restore: save location unknown.", 2000 );
        } else {
            Features::restore_backup( backup, game.save_path, m_pending_conflicts );
            if ( !m_pending_conflicts.empty( ) ) {
                m_open_conflict_modal = true;
            }
        }
    }
    ImGui::SetItemTooltip( "Restore save from backup" );
    ImGui::SameLine( 0.0f, 4.0f );

    if ( ImGui::Button( "Rename", ImVec2( 80.0f, 0 ) ) ) {
        m_pending_rename_game   = game;
        m_pending_rename_backup = backup;
        m_rename_input          = ( it != labels.end( ) ) ? it->second : "";
        m_open_rename_modal     = true;
    }
    ImGui::SetItemTooltip( "Rename this backup" );
    ImGui::SameLine( 0.0f, 4.0f );

    ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.8f, 0.2f, 0.2f, 1.0f ) );
    ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 0.9f, 0.3f, 0.3f, 1.0f ) );
    if ( ImGui::Button( "Delete", ImVec2( 80.0f, 0 ) ) ) {
        if ( fs::remove( backup ) ) {
            auto mutable_labels = labels;
            mutable_labels.erase( backup.filename( ).string( ) );
            Features::save_labels( game.game_name, m_config, mutable_labels );
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

void CDashboardView::render_modals( ) {
    if ( m_open_rename_modal ) {
        m_open_rename_modal = false;
        ImGui::OpenPopup( "Rename Backup" );
    }

    // if ( open_schedule_modal ) {
    //     open_schedule_modal = false;
    //     ImGui::OpenPopup( "Add schedule" );
    // }

    if ( m_open_conflict_modal ) {
        m_open_conflict_modal = false;
        ImGui::OpenPopup( "Resolve conflict(s)" );
    }

    if ( ImGui::BeginPopupModal( "Rename Backup", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) ) {
        ImGui::Text( "%s", m_pending_rename_backup.filename( ).string( ).c_str( ) );
        ImGui::InputText( "Label", &m_rename_input );
        if ( ImGui::Button( "Save" ) ) {
            Features::save_label(
                m_pending_rename_game.game_name, m_config, m_pending_rename_backup.filename( ).string( ),
                m_rename_input );
            ImGui::CloseCurrentPopup( );
        }
        ImGui::SameLine( );
        if ( ImGui::Button( "Cancel" ) ) {
            ImGui::CloseCurrentPopup( );
        }
        ImGui::EndPopup( );
    }

    // if ( ImGui::BeginPopupModal( "Add schedule", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) ) {
    //     ImGui::Checkbox( "Enabled", &schedule_enabled );
    //     ImGui::SliderInt( "Interval (hours)", &schedule_interval_hours, 1, 100 );
    //
    //     ImGui::Separator( );
    //
    //     ImGui::Text( "Found backups:" );
    //     for ( const auto &file : scheduled_files ) {
    //         bool is_included = false;
    //         if ( scheduled_files_selected.contains( file ) ) {
    //             is_included = true;
    //         }
    //         ImGui::Checkbox( ( "##" + file.string( ) ).c_str( ), &is_included );
    //         ImGui::SameLine( );
    //         ImGui::Text( "%s", file.filename( ).string( ).c_str( ) );
    //
    //         if ( is_included ) scheduled_files_selected.insert( file );
    //         else
    //             scheduled_files_selected.erase( file );
    //     }
    //
    //     ScheduleEntry sentry;
    //     const Game &g = pending_schedule_game;
    //
    //     sentry.appid = g.appid;
    //     sentry.game_name = g.game_name;
    //     sentry.save_path = g.save_path;
    //     sentry.enabled = schedule_enabled;
    //     sentry.interval_hours = schedule_interval_hours;
    //     sentry.last_backup_time = -1;
    //
    //     if ( ImGui::Button( "Add" ) ) {
    //         sentry.included_saves =
    //             std::vector<fs::path>( scheduled_files_selected.begin( ), scheduled_files_selected.end( ) );
    //         ctx.scheduler.add_entry( sentry );
    //         ImGui::CloseCurrentPopup( );
    //     }
    //     ImGui::SameLine( );
    //     if ( ImGui::Button( "Done" ) ) {
    //         ImGui::CloseCurrentPopup( );
    //     }
    //     ImGui::EndPopup( );
    // }

    ImGui::SetNextWindowSize( ImVec2( 500, 0 ), ImGuiCond_Always );
    if ( ImGui::BeginPopupModal( "Resolve conflict(s)", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) ) {
        std::vector<int> to_remove;

        for ( size_t i{ }; i < m_pending_conflicts.size( ); i++ ) {
            ImGui::Text( "%s", m_pending_conflicts[i].second.filename( ).string( ).c_str( ) );

            ImGui::PushID( i );
            if ( ImGui::Button( "Keep" ) ) {
                fs::rename( m_pending_conflicts[i].second, m_pending_conflicts[i].first );
                to_remove.push_back( i );
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

        if ( m_pending_conflicts.empty( ) ) {
            ImGui::CloseCurrentPopup( );
        }
        if ( ImGui::Button( "Cancel" ) ) {
            ImGui::CloseCurrentPopup( );
        }
        ImGui::EndPopup( );
    }
}

void CDashboardView::on_result_changed( ) {
    std::shared_lock<std::shared_mutex> lock( m_result.d_mutex );
    m_grouped_games = { };
    m_game_cache.clear( );

    m_grouped_games = m_result.get_grouped( );

    try {
        for ( const auto& game : m_result.games ) {
            GameCache cache;
            if ( !fs::is_directory( game.save_path ) ) continue;
            if ( game.save_path.string( ).contains( ".savemgr-conflict-" ) ) continue;

            if ( game.type != PlatformType::MINECRAFT ) {
                for ( const auto& file : fs::recursive_directory_iterator(
                          game.save_path, fs::directory_options::skip_permission_denied ) ) {
                    if ( !fs::is_regular_file( file ) ) continue;
                    auto ext = file.path( ).extension( ).string( );
                    if ( game.type != PlatformType::CUSTOM ) {
                        if ( std::find( extension_blocklist.begin( ), extension_blocklist.end( ), ext ) !=
                             extension_blocklist.end( ) )
                            continue;
                    }
                    if ( std::find( g_extension_blocklist.begin( ), g_extension_blocklist.end( ), ext ) !=
                         g_extension_blocklist.end( ) )
                        continue;
                    cache.save_files.push_back( file.path( ) );
                }
            } else {
                cache.save_files.push_back( game.save_path );
            }
            cache.backup_count = Features::get_backups( game.game_name, m_config ).size( );
            cache.labels       = Features::load_labels( game.game_name, m_config );
            auto key           = cache_key( game );
            if ( game.type == PlatformType::MINECRAFT ) {
                if ( m_game_cache.contains( key ) ) {
                    m_game_cache[key].save_files.push_back( game.save_path );
                } else {
                    m_game_cache[cache_key( game )] = cache;
                }
            } else {
                m_game_cache[cache_key( game )] = cache;
            }

            try {
                for ( const auto& f : fs::directory_iterator( game.save_path ) ) {
                    if ( f.path( ).string( ).find( ".savemgr-conflict-" ) != std::string::npos ) {
                        // SPDLOG_INFO("writing conflict cache for key: {}", key);
                        // SPDLOG_INFO("path: {}", game.save_path.string());
                        m_game_cache[key].has_conflicts = true;
                        break;
                    }
                }
            } catch ( std::exception& ex ) {
                SPDLOG_ERROR( "conflict iteration error: {}", ex.what( ) );
            }
        }

        for ( const auto& entry : m_result.games ) {
            fs::file_time_type current_max;
            if ( !fs::is_directory( entry.save_path ) ) continue;
            for ( const auto& file :
                  fs::directory_iterator( entry.save_path, fs::directory_options::skip_permission_denied ) ) {
                if ( !fs::exists( file ) ) continue;
                auto t = fs::last_write_time( file );
                if ( fs::is_regular_file( file ) )
                    if ( t > current_max ) current_max = t;
            }
            m_game_last_modified.insert( { entry.game_name, current_max } );
        }
    } catch ( fs::filesystem_error& er ) {
        SPDLOG_ERROR( "dashboard(on_result_changed): {}", er.what( ) );
    }
}
