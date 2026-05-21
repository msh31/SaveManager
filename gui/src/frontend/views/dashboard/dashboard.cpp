#include "dashboard.hpp"
#include "utils/utils.hpp"
#include <constants.hpp>
#include <types.hpp>

#include <features/backup/backup.hpp>

#include "frontend/ui/notifications/notification.hpp"
#include "frontend/ui/widgets.hpp"
#include <frontend/views/backups/backup_view.hpp>

static constexpr const char *ICON_SORT = "\xef\x83\x9c";
static constexpr const char *ICON_FILTER = "\xef\x82\xb0";

void DashboardTab::on_result_changed( RenderContext &ctx ) {
    std::shared_lock<std::shared_mutex> lock( ctx.result.d_mutex );
    grouped_games = { };
    game_cache.clear( );

    grouped_games = ctx.result.get_grouped( );
    last_game_count = ctx.games.size( );

    try {
        for ( const auto &game : ctx.games ) {
            GameCache cache;
            if ( !fs::is_directory( game.save_path ) ) continue;
            if ( game.save_path.string( ).contains( ".savemgr-conflict-" ) ) continue;

            if ( game.type != PlatformType::MINECRAFT ) {
                for ( const auto &file : fs::recursive_directory_iterator(
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
            cache.backup_count = Features::get_backups( game.game_name, ctx.config ).size( );
            cache.labels = Features::load_labels( game.game_name, ctx.config );
            auto key = cache_key( game );
            if ( game.type == PlatformType::MINECRAFT ) {
                if ( game_cache.contains( key ) ) {
                    game_cache[key].save_files.push_back( game.save_path );
                } else {
                    game_cache[cache_key( game )] = cache;
                }
            } else {
                game_cache[cache_key( game )] = cache;
            }

            try {
                for ( const auto &f : fs::directory_iterator( game.save_path ) ) {
                    if ( f.path( ).string( ).find( ".savemgr-conflict-" ) != std::string::npos ) {
                        // SPDLOG_INFO("writing conflict cache for key: {}", key);
                        // SPDLOG_INFO("path: {}", game.save_path.string());
                        game_cache[key].has_conflicts = true;
                        break;
                    }
                }
            } catch ( std::exception &ex ) {
                SPDLOG_ERROR( "conflict iteration error: {}", ex.what( ) );
            }
        }

        for ( const auto &entry : ctx.games ) {
            fs::file_time_type current_max;
            if ( !fs::is_directory( entry.save_path ) ) continue;
            for ( const auto &file :
                  fs::directory_iterator( entry.save_path, fs::directory_options::skip_permission_denied ) ) {
                if ( !fs::exists( file ) ) continue;
                auto t = fs::last_write_time( file );
                if ( fs::is_regular_file( file ) )
                    if ( t > current_max ) current_max = t;
            }
            game_last_modified.insert( { entry.game_name, current_max } );
        }
    } catch ( fs::filesystem_error &er ) {
        SPDLOG_ERROR( "dashboard(on_result_changed): {}", er.what( ) );
    }
}

void DashboardTab::render( const Fonts &fonts, Detection::DetectionResult &result, Config &config) { //,
                           //SaveScheduler &scheduler ) {
    static BackupTab backup;

    if ( ImGui::BeginTabBar( "MyTabBar", ImGuiTabBarFlags_DrawSelectedOverline ) ) {
        if ( ImGui::BeginTabItem( "Games" ) ) {
            std::vector<Game> snapshot;
            {
                std::shared_lock<std::shared_mutex> lock( result.d_mutex );
                snapshot = result.games;
            }

            RenderContext ctx{ result, config, fonts, snapshot}; //, scheduler };
            spinner_frame++;

            if ( refresh_future.valid( ) &&
                 refresh_future.wait_for( std::chrono::seconds( 0 ) ) == std::future_status::ready ) {
                Notify::show_notification( "Save refresh", "Saves refreshed!", 2000 );
                refresh_future.get( );
                on_result_changed( ctx );
            }

            if ( last_game_count != snapshot.size( ) ) {
                on_result_changed( ctx );
            }

            render_toolbar( ctx );
            ImGui::PushStyleVar( ImGuiStyleVar_FrameBorderSize, 2.0f );
            render_game_list( ctx );
            ImGui::PopStyleVar( );
            render_modals( ctx );
            ImGui::EndTabItem( );
        }
        if ( ImGui::BeginTabItem( "Backups" ) ) {
            backup.render( fonts, result, config );
            ImGui::EndTabItem( );
        }
        ImGui::EndTabBar( );
    }
}

void DashboardTab::render_toolbar( RenderContext &ctx ) {
    ImGui::PushFont( ctx.fonts.header );
    ImGui::Text( "Dashboard" );
    ImGui::PopFont( );

    bool is_refreshing =
        refresh_future.valid( ) && refresh_future.wait_for( std::chrono::seconds( 0 ) ) != std::future_status::ready;
    bool is_backing_up =
        backup_future.valid( ) && backup_future.wait_for( std::chrono::seconds( 0 ) ) != std::future_status::ready;

    float sort_width = ImGui::CalcTextSize( "Sort: Alphabetical" ).x + ImGui::GetStyle( ).FramePadding.x * 2;
    float filter_width = ImGui::CalcTextSize( "Filter: Rockstar" ).x + ImGui::GetStyle( ).FramePadding.x * 2;
    float refresh_width = ImGui::CalcTextSize( "Refresh" ).x + ImGui::GetStyle( ).FramePadding.x * 2;
    float backup_width = ImGui::CalcTextSize( "Mass Backup" ).x + ImGui::GetStyle( ).FramePadding.x * 2;
    float spacing = ImGui::GetStyle( ).ItemSpacing.x * 3;

    ImGui::SetNextItemWidth( ImGui::GetContentRegionAvail( ).x - sort_width - refresh_width - backup_width -
                             filter_width - spacing );

    if ( ( ImGui::GetIO( ).KeyCtrl && ImGui::IsKeyPressed( ImGuiKey_F ) ) ) {
        focus_search = true;
    }
    if ( focus_search ) {
        ImGui::SetKeyboardFocusHere( );
        focus_search = false;
    }
    if ( ImGui::InputText( "##search", &search_query ) ) {
        std::transform( search_query.begin( ), search_query.end( ), search_query.begin( ), ::tolower );
    }
    ImGui::SameLine( );

    std::string sort_label =
        sort_mode == SortMode::Alphabetical ? std::format( "{} A-Z", ICON_SORT ) : std::format( "{} Date", ICON_SORT );

    if ( ImGui::Button( sort_label.c_str( ) ) ) {
        sort_mode = sort_mode == SortMode::Alphabetical ? SortMode::Recent : SortMode::Alphabetical;
    }
    ImGui::SameLine( );
    std::string filter_label = platform_filter.has_value( )
                                   ? std::format( "{} {}", ICON_FILTER, get_platform_label( *platform_filter ) )
                                   : std::format( "{} All", ICON_FILTER );
    if ( ImGui::Button( filter_label.c_str( ) ) ) {
        if ( !platform_filter.has_value( ) ) {
            platform_filter = PlatformType::UBISOFT;
        } else {
            switch ( *platform_filter ) {
            case PlatformType::UBISOFT:
                platform_filter = PlatformType::ROCKSTAR;
                break;
            case PlatformType::ROCKSTAR:
                platform_filter = PlatformType::UNREAL;
                break;
            case PlatformType::UNREAL:
                platform_filter = PlatformType::CUSTOM;
                break;
            case PlatformType::CUSTOM:
                platform_filter = std::nullopt;
                break;
            default:
                platform_filter = std::nullopt;
                break;
            }
        }
    }
    ImGui::SameLine( );

    if ( !is_refreshing &&
         ( ImGui::Button( "Refresh" ) || ( ImGui::GetIO( ).KeyCtrl && ImGui::IsKeyPressed( ImGuiKey_R ) ) ) ) {
        {
            std::unique_lock lock( ctx.result.d_mutex );
            ctx.result.games.clear( );
            grouped_games.clear( );
        }
        refresh_future = std::async( std::launch::async, [&result = ctx.result, &config = ctx.config]( ) {
            Detection::find_saves( config, result );
        } );
    }
    ImGui::SetItemTooltip( "Re-runs the detection logic to find new saves" );
    ImGui::SameLine( );
    if ( !is_backing_up && !is_refreshing ) {
        if ( ImGui::Button( "Mass Backup" ) ) {
            backup_future = std::async( std::launch::async, [&result = ctx.result, &config = ctx.config]( ) {
                std::vector<Game> snapshot;
                {
                    std::shared_lock lock( result.d_mutex );
                    snapshot = result.games;
                }
                Features::backup_all_games( snapshot, config );
                Notify::show_notification("Mass Backup", "Succesfully backed up all gamesaves!", 1500);
            } );
        }
        ImGui::SetItemTooltip( "Creates a backup of all games found!" );
    } else {
        char spin_char = Widgets::spinner[( spinner_frame / 10 ) % 4];
        std::string loading_text = std::string( "Loading.." ) + spin_char;
        ImGui::Text( "%s", loading_text.c_str( ) );
    }

    ImGui::Dummy( ImVec2( 0.0f, 5.0f ) );
}

void DashboardTab::render_game_list( RenderContext &ctx ) {
    auto sorted = grouped_games;
    switch ( sort_mode ) {
    case SortMode::Recent:
        std::sort( sorted.begin( ), sorted.end( ), [&]( const std::vector<int> &a, const std::vector<int> &b ) {
            return game_last_modified[ctx.games[a[0]].game_name] > game_last_modified[ctx.games[b[0]].game_name];
        } );
        break;
    case SortMode::Alphabetical:
        std::sort( sorted.begin( ), sorted.end( ), [&]( const std::vector<int> &a, const std::vector<int> &b ) {
            return ctx.games[a[0]].game_name < ctx.games[b[0]].game_name;
        } );
        break;
    }

    enumerate( sorted, [&]( int gi, auto &group ) {
        if ( platform_filter.has_value( ) && ctx.games[group[0]].type != *platform_filter ) return;

        const Game &primary = ctx.games[group[0]];
        std::string game_name = primary.game_name;

        std::transform( game_name.begin( ), game_name.end( ), game_name.begin( ), ::tolower );
        if ( !search_query.empty( ) ) {
            if ( game_name.find( search_query ) == std::string::npos ) {
                return;
            }
        }
        render_game_row( ctx, group, static_cast<int>( gi ) );
        ImGui::Dummy( ImVec2( 0, 6.0f ) );
    } );
}

void DashboardTab::render_game_row( RenderContext &ctx, const std::vector<int> &group, int gi ) {
    const Game &primary = ctx.games[group[0]];

    bool &not_collapsed = card_collapsed[cache_key( primary )];
    bool &bk_collapsed = backups_collapsed[cache_key( primary )];
    const char *chevron_b = bk_collapsed ? "▶" : "▼";

    std::vector<std::pair<fs::path, const Game *>> files = { };
    auto &cache = game_cache[cache_key( primary )];
    int save_count = cache.save_files.size( );
    int backup_count = cache.backup_count;
    auto labels = cache.labels;
    // SPDLOG_INFO("reading conflict cache for key: {} has_conflicts: {}", cache_key(primary), cache.has_conflicts);
    bool has_conflicts = cache.has_conflicts;

    for ( const auto &path : cache.save_files ) {
        files.emplace_back( path, &primary );
    }

    auto selectable_id = std::format( "##gamename_{}", gi );
    std::string right_text =
        std::format( "{} | {} saves | {} backups", get_platform_label( primary.type ), save_count, backup_count );
    Widgets::begin_game_card( selectable_id.c_str( ), ctx.fonts, not_collapsed, primary.game_name.c_str( ),
                              right_text.c_str( ) );

    if ( not_collapsed ) {
        if ( save_count > 0 ) {
            if ( primary.type != PlatformType::MINECRAFT ) ImGui::TextDisabled( "SAVE FILES" );
            else
                ImGui::TextDisabled( "WORLDS" );

            // todo cache this
            fs::path undo_dir = paths::backup_dir( ) / sanitize_filename( primary.game_name ) / "undo.zip";
            bool has_undo = false;
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
                backup_future = std::async( std::launch::async, [this, primary, files]( ) {
                    if ( Features::backup_game_files( primary, files ) ) {
                        Notify::show_notification( "Backup Created", "A backup has been for all saves!", 1500 );
                    } else {
                        Notify::show_notification( "Backup Creation",
                                                   "Failed to create backup! Please refer to the logfile!", 2000 );
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
                    pending_conflicts.clear( );
                    for ( const auto &f : fs::directory_iterator( primary.save_path ) ) {
                        auto full = f.path( ).string( );
                        auto pos = full.find( ".savemgr-conflict-" );
                        fs::path original = full.substr( 0, pos );
                        pending_conflicts.push_back( { original, f.path( ) } );
                    }
                    open_conflict_modal = true;
                }
            }
            ImGui::SameLine( );
            if ( has_undo ) {
                if ( ImGui::Button( "Undo last restore" ) ) {
                    Features::restore_backup( undo_dir, primary.save_path, pending_conflicts );
                    fs::remove( undo_dir );
                }
            }

            for ( auto &save : files ) {
                if ( !fs::exists( save.first ) ) continue;
                if ( save.first.string( ).contains( ".savemgr-conflict-" ) ) continue;

                ImGui::Separator( );
                render_save_row( ctx, save.first, *save.second );
                ImGui::Separator( );
            }
        } else
            ImGui::TextDisabled( "Game detected but no saves were found!" );
        if ( backup_count > 0 ) {
            if ( ImGui::Selectable( "##backups", false, ImGuiSelectableFlags_None, ImVec2( 0, 30 ) ) ) {
                bk_collapsed = !bk_collapsed;
            }
            ImGui::SameLine( 8.0f );

            ImGui::PushFont( ctx.fonts.bold );
            ImGui::TextColored( ImColor( 198, 97, 63 ).Value, "%s", chevron_b );
            ImGui::PopFont( );
            ImGui::SameLine( );
            ImGui::PushFont( ctx.fonts.medium );
            ImGui::Text( "BACKUPS" );
            ImGui::PopFont( );

            if ( !bk_collapsed ) {
                auto backups = Features::get_backups( primary.game_name, ctx.config );
                for ( auto &backup : backups ) {
                    ImGui::Separator( );
                    render_backup_row( ctx, backup, primary, labels );
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

    Widgets::end_game_card( );
}

void DashboardTab::render_save_row( RenderContext &ctx, const fs::path &save_file, const Game &game ) {
    ImGui::PushID( save_file.string( ).c_str( ) );

    std::string date_text = std::format( "{:%d/%m/%y %H:%M} | ", fs::last_write_time( save_file ) );
    float date_width = ImGui::CalcTextSize( date_text.c_str( ) ).x;

    std::string size_text;
    if ( game.type != PlatformType::MINECRAFT ) {
        auto b_size = fs::file_size( save_file ) / 1024;
        size_text = std::format( "{}KB  ", b_size );
    }

    float size_width = ImGui::CalcTextSize( size_text.c_str( ) ).x;
    float total_width = date_width + size_width + Widgets::btn_width * 1 + Widgets::button_spacing * 5;

    if ( game.show_parent_path ) {
        ImGui::Text( "%s", ( save_file.parent_path( ).filename( ) / save_file.filename( ) ).string( ).c_str( ) );
    } else {
        ImGui::Text( "%s", save_file.filename( ).string( ).c_str( ) );
    }

    ImGui::SameLine( ImGui::GetContentRegionMax( ).x - total_width );

    ImGui::TextDisabled( "%s", date_text.c_str( ) );
    ImGui::SameLine( 0.0f, Widgets::button_spacing );
    ImGui::TextDisabled( "%s", size_text.c_str( ) );
    ImGui::SameLine( 0.0f, Widgets::button_spacing );

    ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 3.0f, 3.0f ) );
    if ( ImGui::Button( "Backup", ImVec2( Widgets::btn_width, 0 ) ) ) {
        backup_future = std::async( std::launch::async, [game, save_file, &config = ctx.config]( ) {
            Features::backup_game( game, save_file, config );
        } );
    }
    ImGui::SetItemTooltip( "Create a backup of this save" );
    ImGui::PopStyleVar( );
    ImGui::PopID( );
}

void DashboardTab::render_backup_row( RenderContext &ctx, const fs::path &backup, const Game &game,
                                      const std::unordered_map<std::string, std::string> &labels ) {
    if ( backup.filename( ) == "undo.zip" ) return;
    ImGui::PushID( backup.string( ).c_str( ) );
    if ( !fs::exists( backup ) ) {
        ImGui::PopID( );
        return;
    }

    auto it = labels.find( backup.filename( ).string( ) );
    std::string display = ( it != labels.end( ) ) ? it->second : backup.filename( ).string( );

    std::string date_text = std::format( "{:%d/%m/%y %H:%M} | ", fs::last_write_time( backup ) );
    float date_width = ImGui::CalcTextSize( date_text.c_str( ) ).x;
    auto b_size = fs::file_size( backup ) / 1024;

    std::string size_text = std::format( "{}KB  ", b_size );
    float size_width = ImGui::CalcTextSize( size_text.c_str( ) ).x;

    float total_width = date_width + size_width + Widgets::btn_width * 3 + Widgets::button_spacing * 5;

    ImGui::Text( "%s", display.c_str( ) );
    ImGui::SameLine( ImGui::GetContentRegionMax( ).x - total_width );

    ImGui::TextDisabled( "%s", date_text.c_str( ) );
    ImGui::SameLine( 0.0f, Widgets::button_spacing );
    ImGui::TextDisabled( "%s", size_text.c_str( ) );
    ImGui::SameLine( 0.0f, Widgets::button_spacing );

    ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 3.0f, 3.0f ) );
    if ( ImGui::Button( "Restore", ImVec2( Widgets::btn_width, 0 ) ) ) {
        if ( game.save_path.empty( ) ) {
            Notify::show_notification( "Restore", "Cannot restore: save location unknown.", 2000 );
        } else {
            Features::restore_backup( backup, game.save_path, pending_conflicts );
            if ( !pending_conflicts.empty( ) ) {
                open_conflict_modal = true;
            }
        }
    }
    ImGui::SetItemTooltip( "Restore save from backup" );
    ImGui::SameLine( 0.0f, Widgets::button_spacing );

    if ( ImGui::Button( "Rename", ImVec2( Widgets::btn_width, 0 ) ) ) {
        pending_rename_game = game;
        pending_rename_backup = backup;
        rename_input = ( it != labels.end( ) ) ? it->second : "";
        open_rename_modal = true;
    }
    ImGui::SetItemTooltip( "Rename this backup" );
    ImGui::SameLine( 0.0f, Widgets::button_spacing );

    ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.8f, 0.2f, 0.2f, 1.0f ) );
    ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 0.9f, 0.3f, 0.3f, 1.0f ) );
    if ( ImGui::Button( "Delete", ImVec2( Widgets::btn_width, 0 ) ) ) {
        if ( fs::remove( backup ) ) {
            auto mutable_labels = labels;
            mutable_labels.erase( backup.filename( ).string( ) );
            Features::save_labels( game.game_name, ctx.config, mutable_labels );
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

void DashboardTab::render_modals( RenderContext &ctx ) {
    if ( open_rename_modal ) {
        open_rename_modal = false;
        ImGui::OpenPopup( "Rename Backup" );
    }

    // if ( open_schedule_modal ) {
    //     open_schedule_modal = false;
    //     ImGui::OpenPopup( "Add schedule" );
    // }

    if ( open_conflict_modal ) {
        open_conflict_modal = false;
        ImGui::OpenPopup( "Resolve conflict(s)" );
    }

    if ( ImGui::BeginPopupModal( "Rename Backup", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) ) {
        ImGui::Text( "%s", pending_rename_backup.filename( ).string( ).c_str( ) );
        ImGui::InputText( "Label", &rename_input );
        if ( ImGui::Button( "Save" ) ) {
            Features::save_label( pending_rename_game.game_name, ctx.config,
                                  pending_rename_backup.filename( ).string( ), rename_input );
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

        for ( size_t i{ }; i < pending_conflicts.size( ); i++ ) {
            ImGui::Text( "%s", pending_conflicts[i].second.filename( ).string( ).c_str( ) );

            ImGui::PushID( i );
            if ( ImGui::Button( "Keep" ) ) {
                fs::rename( pending_conflicts[i].second, pending_conflicts[i].first );
                to_remove.push_back( i );
            }
            ImGui::PopID( );
            ImGui::SetItemTooltip( "Overwrite the newer save with this restored from backup file" );
            ImGui::SameLine( );
            ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.8f, 0.2f, 0.2f, 1.0f ) );
            ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 0.9f, 0.3f, 0.3f, 1.0f ) );
            ImGui::PushID( i );
            if ( ImGui::Button( "Delete" ) ) {
                fs::remove( pending_conflicts[i].second );
                to_remove.push_back( i );
            }
            ImGui::PopID( );
            ImGui::PopStyleColor( 2 );
        }

        for ( int i = to_remove.size( ) - 1; i >= 0; i-- ) {
            pending_conflicts.erase( pending_conflicts.begin( ) + to_remove[i] );
        }

        if ( pending_conflicts.empty( ) ) {
            ImGui::CloseCurrentPopup( );
        }
        if ( ImGui::Button( "Cancel" ) ) {
            ImGui::CloseCurrentPopup( );
        }
        ImGui::EndPopup( );
    }
}
