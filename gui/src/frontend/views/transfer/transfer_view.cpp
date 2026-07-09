#include "transfer_view.hpp"
#include <utils/utils.hpp>

#include <detection/detection.hpp>
#include <features/features.hpp>

#include <backend/font_manager/font_manager.hpp>

#include <frontend/components/spinner.hpp>
#include <frontend/notification/notification.hpp>

void CTransferView::on_enter( ) {
    if ( m_result.empty( ) )
        m_detection_future = std::async( std::launch::async, [this] {
            auto result = Detection::find_saves( m_blacklist, m_translations );
            std::lock_guard lock( m_result_mutex );
            m_result = std::move( result );
        } );
}

void CTransferView::render( ) {
    bool detection_done = m_detection_future.valid( ) &&
                          m_detection_future.wait_for( std::chrono::seconds( 0 ) ) == std::future_status::ready;

    if ( detection_done ) {
        m_games_snapshot = m_result;
        m_detection_future.get( );
    }

    if ( !m_initialized ) {
        m_remote = std::make_unique<CRemoteTransfer>( );

        m_dest_addr = m_config.sftp.dest_addr;
        m_username = m_config.sftp.username;
        m_password = m_config.sftp.password;
        m_pubkey = m_config.sftp.pubkey.string( );
        m_privkey = m_config.sftp.privkey.string( );
        m_key_passphrase = m_config.sftp.key_passphrase;
        m_initialized = true;
    }

    bool is_transferring =
        m_future.valid( ) && m_future.wait_for( std::chrono::seconds( 0 ) ) != std::future_status::ready;
    bool is_connecting = m_connect_future.valid( ) &&
                         m_connect_future.wait_for( std::chrono::seconds( 0 ) ) != std::future_status::ready;

    float file_progress = 0.0f;
    float overall_progress = 0.0f;
    bool transferring = is_transferring && m_remote;

    if ( transferring ) {
        if ( m_remote->m_total_bytes > 0 )
            file_progress = (float)m_remote->m_bytes_transferred / (float)m_remote->m_total_bytes;
        if ( m_total_files > 0 )
            overall_progress = ( (float)m_current_file_index + file_progress ) / (float)m_total_files;
    } else if ( m_was_transferring && !transferring ) {
        Notify::show_notification( "Transfer Complete", "All files transferred successfully!", 2000 );
    }

    m_was_transferring = transferring;

    if ( m_connect_future.valid( ) &&
         m_connect_future.wait_for( std::chrono::seconds( 0 ) ) == std::future_status::ready ) {
        bool success = m_connect_future.get( );
        if ( success ) {
            m_connected = true;
            m_current_remote_path = "/home/" + m_config.sftp.username; // TODO: allow custom start location..
            m_remote_entries = m_remote->list_directory( m_current_remote_path );
            Notify::show_notification( "SFTP Connection", "Connected!", 2000 );
        } else {
            Notify::show_notification( "SFTP Connection", "Failed to connect!", 2000 );
        }
    }

    ImGui::PushFont( CFontManager::get( ).get_font( "jbm_bold" ).value_or( nullptr ) );
    ImGui::Text( "Save Transfer" );
    ImGui::PopFont( );
    ImGui::Separator( );

    ImGui::BeginChild(
        "##transfer_wrapper", ImVec2( 0, ImGui::GetContentRegionAvail( ).y ), ImGuiChildFlags_None,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground );

    float window_width = ImGui::GetWindowSize( ).x;
    float top_height = m_use_password_auth ? 290.0f : 370.0f;
    float half = ( window_width - 20.0f ) / 2.0f;

    ImGui::BeginChild( "##server", ImVec2( half, top_height ), ImGuiChildFlags_Borders );
    ImGui::PushFont( CFontManager::get( ).get_font( "jbm_med" ).value_or( nullptr ) );
    ImGui::Text( "Server" );
    ImGui::PopFont( );

    ImGui::SetNextItemWidth( 250.0f );
    ImGui::InputText( "Address", &m_dest_addr );
    ImGui::Dummy( ImVec2( 0.0f, 5.0f ) );

    ImGui::PushFont( CFontManager::get( ).get_font( "jbm_med" ).value_or( nullptr ) );
    ImGui::Text( "Authentication" );
    ImGui::PopFont( );

    if ( ImGui::RadioButton( "Password", m_use_password_auth ) ) m_use_password_auth = true;
    ImGui::SameLine( );
    if ( ImGui::RadioButton( "SSH Key", !m_use_password_auth ) ) m_use_password_auth = false;

    ImGui::SetNextItemWidth( 120.0f );
    ImGui::InputText( "Username##user", &m_username );

    if ( m_use_password_auth ) {
        ImGui::SetNextItemWidth( 120.0f );
        ImGui::InputText( "Password##user", &m_password, ImGuiInputTextFlags_Password );
    } else {
        ImGui::SetNextItemWidth( 250.0f );
        ImGui::InputText( "Public key", &m_pubkey );
        ImGui::SetItemTooltip( "Path to your public ssh key in full" );
        ImGui::SetNextItemWidth( 250.0f );
        ImGui::InputText( "Private key", &m_privkey );
        ImGui::SetItemTooltip( "Path to your private ssh key in full" );
        ImGui::SetNextItemWidth( 250.0f );
        ImGui::InputText( "Key passphrase (Optional)", &m_key_passphrase, ImGuiInputTextFlags_Password );
        ImGui::SetItemTooltip( "Your passphrase for the ssh key" );
    }

    float status_y = top_height - 45.0f;
    ImGui::SetCursorPosY( status_y );
    if ( !is_connecting && !m_connected ) {
        if ( ImGui::Button( "Connect" ) ) {
            m_connect_future = std::async(
                std::launch::async,
                [this, r = m_remote.get( ), addr = m_dest_addr, auth = m_use_password_auth,
                 pass = m_key_passphrase]( ) -> bool { return r->connect( addr, m_config, auth, pass ); } );
        }
    } else if ( !m_connected ) {
        Spinner::render( );
    }
    if ( m_connected && !is_transferring ) {
        ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.8f, 0.2f, 0.2f, 1.0f ) );
        ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 0.9f, 0.3f, 0.3f, 1.0f ) );
        if ( ImGui::Button( "Disconnect" ) ) {
            m_connected = false;
            m_remote_entries = { };
            m_remote->disconnect( );
            Notify::show_notification( "SFTP Connection", "Disconnected from server!", 2000 );
        }
        ImGui::PopStyleColor( 2 );
    }
    ImGui::SameLine( );
    if ( m_connected ) {
        ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.3f, 1.0f, 0.3f, 1.0f ) );
        ImGui::BulletText( "Connected to %s", m_dest_addr.c_str( ) );
        ImGui::PopStyleColor( );
    } else {
        ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.0f, 0.3f, 0.3f, 1.0f ) );
        ImGui::BulletText( "Not connected" );
        ImGui::PopStyleColor( );
    }
    ImGui::EndChild( );

    ImGui::SameLine( 0.0f, 10.0f );

    ImGui::BeginChild( "##progress", ImVec2( half, top_height ), ImGuiChildFlags_Borders );
    ImGui::Text( "File:" );
    ImGui::SameLine( 140.0f );
    ImGui::PushStyleColor( ImGuiCol_PlotHistogram, ImVec4( 0.26f, 0.59f, 0.98f, 1.0f ) );
    ImGui::ProgressBar(
        file_progress, ImVec2( 300.0f, 0.0f ),
        transferring ? std::format( "{}%", (int)( file_progress * 100 ) ).c_str( ) : "Idle" );
    ImGui::PopStyleColor( );

    ImGui::Text( "Overall:" );
    ImGui::SameLine( 140.0f );
    ImGui::PushStyleColor( ImGuiCol_PlotHistogram, ImVec4( 0.26f, 0.59f, 0.98f, 1.0f ) );
    ImGui::ProgressBar(
        overall_progress, ImVec2( 300.0f, 0.0f ),
        transferring ? std::format( "{}%", (int)( overall_progress * 100 ) ).c_str( ) : "Idle" );
    ImGui::PopStyleColor( );

    ImGui::SetCursorPosY( status_y );
    if ( is_connecting ) ImGui::BeginDisabled( true );
    if ( ImGui::Button( "Save configuration" ) ) {
        m_config.sftp.dest_addr = fs::path( m_dest_addr ).string( );
        m_config.sftp.username = m_username;
        m_config.sftp.password = m_password;
        m_config.sftp.pubkey = fs::path( m_pubkey );
        m_config.sftp.privkey = fs::path( m_privkey );
        m_config.sftp.key_passphrase = m_key_passphrase;
        m_config.sftp.auth_pw = m_use_password_auth;
        m_config.save( );
        Notify::show_notification( "Config Saved!", "Settings saved successfully!", 1500 );
    }
    if ( is_connecting ) ImGui::EndDisabled( );
    ImGui::EndChild( );
    ImGui::Dummy( ImVec2( 0, 8.0f ) );

    float bottom_height = ImGui::GetContentRegionAvail( ).y;

    ImGui::BeginChild( "##transfer_local", ImVec2( half, bottom_height ), ImGuiChildFlags_Borders );
    ImGui::PushFont( CFontManager::get( ).get_font( "jbm_med" ).value_or( nullptr ) );
    ImGui::Text( "Local" );
    ImGui::PopFont( );

    int local_selected_count = 0;
    for ( bool b : m_selected_backups )
        if ( b ) local_selected_count++;

    ImGui::BeginDisabled( !m_connected || local_selected_count == 0 || is_transferring );
    if ( ImGui::Button( "Upload" ) ) {
        auto selected_paths = std::views::zip( m_backups, m_selected_backups ) |
                              std::views::filter( []( const auto& pair ) { return std::get<1>( pair ); } ) |
                              std::views::transform( []( const auto& pair ) { return std::get<0>( pair ); } ) |
                              std::ranges::to<std::vector>( );

        if ( !selected_paths.empty( ) ) {
            m_total_files = selected_paths.size( );
            m_current_file_index = 0;

            m_future = std::async(
                std::launch::async, [this, r = m_remote.get( ), cr = m_current_remote_path, selected_paths]( ) {
                    enumerate( selected_paths, [&]( int gi, auto& path ) {
                        m_current_file_index = static_cast<int>( gi );
                        if ( !r->upload_file( path, cr, m_config ) ) {
                            auto str = std::format( "Failed to upload: {}", path.string( ) );
                            Notify::show_notification( "Upload", str, 2500 );
                        } else {
                            auto str = std::format( "Uploaded {}!", path.string( ) );
                            Notify::show_notification( "Upload", str, 2500 );
                        }
                    } );
                } );
        }
    }
    ImGui::EndDisabled( );

    ImGui::SameLine( );
    ImGui::Text( "(%d selected)", local_selected_count );

    float content_height = ImGui::GetContentRegionAvail( ).y - 10.0f;

    auto groups = get_grouped( m_games_snapshot );
    std::vector<std::string> game_names;
    for ( const auto& group : groups )
        game_names.push_back( m_games_snapshot[group[0]].game_name );

    if ( !game_names.empty( ) ) {
        if ( m_selected_game_idx >= (int)game_names.size( ) ) m_selected_game_idx = 0;

        ImGui::SetNextItemWidth( -FLT_MIN );
        if ( ImGui::BeginCombo( "##game", game_names[m_selected_game_idx].c_str( ) ) ) {
            enumerate( game_names, [&]( int gi, auto& name ) {
                bool is_selected = ( m_selected_game_idx == static_cast<int>( gi ) );
                if ( ImGui::Selectable( name.c_str( ), is_selected ) ) {
                    m_selected_game_idx = static_cast<int>( gi );
                    m_backups = Features::get_backups( m_games_snapshot[groups[static_cast<int>( gi )][0]].game_name );
                    m_selected_backups.assign( m_backups.size( ), false );
                }
                if ( is_selected ) ImGui::SetItemDefaultFocus( );
            } );
            ImGui::EndCombo( );
        }

        if ( !m_backups.empty( ) ) {
            if ( ImGui::BeginListBox( "##backups", ImVec2( -FLT_MIN, content_height ) ) ) {
                enumerate( m_backups, [&]( int gi, auto& path ) {
                    if ( path.filename( ) == "undo.zip" ) return;
                    std::string label = std::format( "{}##{}", path.filename( ).string( ), static_cast<int>( gi ) );
                    if ( ImGui::Selectable(
                             label.c_str( ), m_selected_backups[static_cast<int>( gi )],
                             ImGuiSelectableFlags_AllowDoubleClick ) ) {
                        m_selected_backups[static_cast<int>( gi )] = !m_selected_backups[static_cast<int>( gi )];
                    }
                } );
                ImGui::EndListBox( );
            }
        } else {
            ImGui::TextDisabled( "No backups found" );
        }
    } else {
        ImGui::TextDisabled( "No games detected" );
    }
    ImGui::EndChild( );

    ImGui::SameLine( 0.0f, 10.0f );

    ImGui::BeginChild( "##transfer_remote", ImVec2( half, bottom_height ), ImGuiChildFlags_Borders );
    ImGui::PushFont( CFontManager::get( ).get_font( "jbm_med" ).value_or( nullptr ) );
    ImGui::Text( "Remote" );
    ImGui::PopFont( );

    if ( !m_connected ) {
        ImGui::TextDisabled( "Connect to browse remote server" );
    } else {
        bool has_remote_selection = m_selected_remote_idx >= 0 && m_selected_remote_idx < (int)m_remote_entries.size( );
        bool is_file_selected = has_remote_selection && !m_remote_entries[m_selected_remote_idx].is_directory;

        ImGui::BeginDisabled( !is_file_selected || is_transferring );
        if ( ImGui::Button( "Download" ) ) {
            if ( m_current_remote_path.empty( ) ) {
                Notify::show_notification(
                    "Download Failure", "Failed to download file: No remote path specified!", 2000 );
                ImGui::EndDisabled( );
                return;
            }
            std::string path = m_current_remote_path + ( m_current_remote_path.back( ) == '/' ? "" : "/" ) +
                               m_remote_entries[m_selected_remote_idx].name;
            ;
            if ( !m_remote->download_file( path, m_config ) ) {
                auto str = std::format( "Failed to download: {}", path );
                Notify::show_notification( "Download", str, 2500 );
            } else {
                auto str = std::format( "Downloaded {}!", path );
                Notify::show_notification( "Download", str, 2500 );
            }
        }
        ImGui::EndDisabled( );

        ImGui::SameLine( );
        if ( has_remote_selection ) ImGui::Text( "%s", m_remote_entries[m_selected_remote_idx].name.c_str( ) );
        else
            ImGui::TextDisabled( "(no selection)" );

        ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetStyle( ).Colors[ImGuiCol_TextDisabled] );
        ImGui::Text( "Path:" );
        ImGui::PopStyleColor( );
        ImGui::SameLine( );
        ImGui::Text( "%s", m_current_remote_path.c_str( ) );

        float remote_content_height = ImGui::GetContentRegionAvail( ).y - 10.0f;

        if ( ImGui::BeginListBox( "##remote_entries", ImVec2( -FLT_MIN, remote_content_height ) ) ) {
            if ( m_current_remote_path != "/" ) {
                if ( is_transferring ) ImGui::BeginDisabled( true );
                if ( ImGui::Selectable( "..##parent", false ) ) {
                    m_current_remote_path = fs::path( m_current_remote_path ).parent_path( ).string( );
                    if ( m_current_remote_path.empty( ) ) m_current_remote_path = "/";
                    m_remote_entries = m_remote->list_directory( m_current_remote_path );
                    m_selected_remote_idx = -1;
                }
                if ( is_transferring ) ImGui::EndDisabled( );
            }

            enumerate( m_remote_entries, [&]( int gi, auto& entry ) {
                if ( entry.name == "." || entry.name == ".." ) return;

                std::string prefix = entry.is_directory ? "[DIR] " : "[FILE] ";
                std::string label = std::format( "{}{}##{}", prefix, entry.name, static_cast<int>( gi ) );
                if ( is_transferring ) ImGui::BeginDisabled( true );
                if ( ImGui::Selectable(
                         label.c_str( ), m_selected_remote_idx == static_cast<int>( gi ),
                         ImGuiSelectableFlags_AllowDoubleClick ) ) {
                    if ( ImGui::IsMouseDoubleClicked( 0 ) && entry.is_directory ) {
                        if ( m_current_remote_path.empty( ) ) {
                            SPDLOG_ERROR( "Failed to download file: No remote path specified!" );
                            return;
                        }
                        m_current_remote_path =
                            m_current_remote_path + ( m_current_remote_path.back( ) == '/' ? "" : "/" ) + entry.name;
                        m_remote_entries = m_remote->list_directory( m_current_remote_path );
                        m_selected_remote_idx = -1;
                    } else {
                        m_selected_remote_idx = static_cast<int>( gi );
                    }
                }
                if ( is_transferring ) ImGui::EndDisabled( );
            } );
            ImGui::EndListBox( );
        }
    }
    ImGui::EndChild( );
    ImGui::EndChild( );
}
