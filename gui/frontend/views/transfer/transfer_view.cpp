#include "transfer_view.hpp"
#include <config/config.hpp>
#include <utils/utils.hpp>
#include <backup/backup.hpp>
#include <detection/detection.hpp>

#include <backend/font_manager/font_manager.hpp>

#include <frontend/childguard.hpp>
#include <frontend/components/spinner.hpp>
#include <frontend/notification/notification.hpp>

void CTransferView::on_enter( ) { CDetectionService::get( ).ensure_started( ); }

void CTransferView::render( ) {
    m_queue.update( );

    if ( CDetectionService::get( ).generation( ) != m_seen_generation ) {
        m_seen_generation = CDetectionService::get( ).generation( );
        m_games_snapshot = CDetectionService::get( ).snapshot( );
    }

    if ( !m_initialized ) {
        m_remote = std::make_shared<CRemoteTransfer>( );

        m_dest_addr = CConfig::get( ).sftp.dest_addr;
        m_username = CConfig::get( ).sftp.username;
        m_password = CConfig::get( ).sftp.password;
        m_pubkey = CConfig::get( ).sftp.pubkey.string( );
        m_privkey = CConfig::get( ).sftp.privkey.string( );
        m_key_passphrase = CConfig::get( ).sftp.key_passphrase;
        m_initialized = true;
    }

    bool is_connecting = m_connecting;
    bool uploading = m_transfer_handle.has_value( );
    bool is_transferring = uploading || m_downloading;

    float file_progress = 0.0f;
    float overall_progress = 0.0f;
    bool transferring = is_transferring && m_remote;

    if ( transferring ) {
        if ( m_remote->m_total_bytes > 0 )
            file_progress = (float)m_remote->m_bytes_transferred / (float)m_remote->m_total_bytes;
        overall_progress = uploading ? m_transfer_handle->progress( ) : file_progress;
    }

    ImGui::PushFont( CFontManager::get( ).get_font( "jbm_bold" ).value_or( nullptr ) );
    ImGui::Text( "Save Transfer" );
    ImGui::PopFont( );
    ImGui::Separator( );

    ChildGuard wrapper(
        "##transfer_wrapper", ImVec2( 0, ImGui::GetContentRegionAvail( ).y ), ImGuiChildFlags_None,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground );

    float window_width = ImGui::GetWindowSize( ).x;
    float top_height = m_use_password_auth ? 290.0f : 370.0f;
    float half = ( window_width - 20.0f ) / 2.0f;

    {
        ChildGuard server( "##server", ImVec2( half, top_height ), ImGuiChildFlags_Borders );
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
                m_connecting = true;
                m_queue.run<bool>(
                    [remote = m_remote, addr = m_dest_addr, auth = m_use_password_auth,
                     pass = m_key_passphrase]( TaskControl& ) {
                        return remote->connect( addr, CConfig::get( ), auth, pass );
                    },
                    [this]( bool success ) {
                        m_connecting = false;
                        if ( success ) {
                            m_connected = true;
                            m_current_remote_path =
                                "/home/" + CConfig::get( ).sftp.username; // TODO: allow custom start location..
                            m_remote_entries = m_remote->list_directory( m_current_remote_path );
                            Notify::show_notification( "SFTP Connection", "Connected!", 2000 );
                        } else {
                            Notify::show_notification( "SFTP Connection", "Failed to connect!", 2000 );
                        }
                    },
                    [this]( const std::exception& ex ) {
                        m_connecting = false;
                        Notify::show_notification( "SFTP Connection", ex.what( ), 2000 );
                    } );
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
    }

    ImGui::SameLine( 0.0f, 10.0f );

    {
        ChildGuard progress( "##progress", ImVec2( half, top_height ), ImGuiChildFlags_Borders );
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

        if ( uploading ) {
            ImGui::SameLine( );
            if ( ImGui::SmallButton( "Cancel" ) ) m_transfer_handle->request_cancel( );
        }

        float status_y = top_height - 45.0f;
        ImGui::SetCursorPosY( status_y );
        if ( is_connecting ) ImGui::BeginDisabled( true );
        if ( ImGui::Button( "Save configuration" ) ) {
            CConfig::get( ).sftp.dest_addr = fs::path( m_dest_addr ).string( );
            CConfig::get( ).sftp.username = m_username;
            CConfig::get( ).sftp.password = m_password;
            CConfig::get( ).sftp.pubkey = fs::path( m_pubkey );
            CConfig::get( ).sftp.privkey = fs::path( m_privkey );
            CConfig::get( ).sftp.key_passphrase = m_key_passphrase;
            CConfig::get( ).sftp.auth_pw = m_use_password_auth;
            CConfig::get( ).save( );
            Notify::show_notification( "Config Saved!", "Settings saved successfully!", 1500 );
        }
        if ( is_connecting ) ImGui::EndDisabled( );
    }
    ImGui::Dummy( ImVec2( 0, 8.0f ) );

    float bottom_height = ImGui::GetContentRegionAvail( ).y;

    {
        ChildGuard local( "##transfer_local", ImVec2( half, bottom_height ), ImGuiChildFlags_Borders );
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
                m_transfer_handle = m_queue.run<int>(
                    [remote = m_remote, cr = m_current_remote_path, selected_paths]( TaskControl& control ) {
                        utils::enumerate( selected_paths, [&]( int gi, auto& path ) {
                            if ( control.cancel_requested.load( ) ) throw TaskCancelled{ };
                            if ( !remote->upload_file( path, cr, CConfig::get( ) ) ) {
                                auto str = std::format( "Failed to upload: {}", path.string( ) );
                                Notify::show_notification( "Upload", str, 2500 );
                            } else {
                                auto str = std::format( "Uploaded {}!", path.string( ) );
                                Notify::show_notification( "Upload", str, 2500 );
                            }
                            control.progress.store( (float)( gi + 1 ) / (float)selected_paths.size( ) );
                        } );
                        return 0;
                    },
                    [this]( int ) {
                        m_transfer_handle = std::nullopt;
                        Notify::show_notification( "Transfer Complete", "All files transferred successfully!", 2000 );
                    },
                    [this]( const std::exception& ex ) {
                        m_transfer_handle = std::nullopt;
                        Notify::show_notification( "Upload", ex.what( ), 2500 );
                    } );
            }
        }
        ImGui::EndDisabled( );

        ImGui::SameLine( );
        ImGui::Text( "(%d selected)", local_selected_count );

        float content_height = ImGui::GetContentRegionAvail( ).y - 10.0f;

        auto groups = utils::get_grouped( m_games_snapshot );
        std::vector<std::string> game_names;
        for ( const auto& group : groups )
            game_names.push_back( m_games_snapshot[group[0]].game_name );

        if ( !game_names.empty( ) ) {
            if ( m_selected_game_idx >= (int)game_names.size( ) ) m_selected_game_idx = 0;

            ImGui::SetNextItemWidth( -FLT_MIN );
            if ( ImGui::BeginCombo( "##game", game_names[m_selected_game_idx].c_str( ) ) ) {
                utils::enumerate( game_names, [&]( int gi, auto& name ) {
                    bool is_selected = ( m_selected_game_idx == static_cast<int>( gi ) );
                    if ( ImGui::Selectable( name.c_str( ), is_selected ) ) {
                        m_selected_game_idx = static_cast<int>( gi );
                        m_backups =
                            Backup::get_backups( m_games_snapshot[groups[static_cast<int>( gi )][0]].game_name );
                        m_selected_backups.assign( m_backups.size( ), false );
                    }
                    if ( is_selected ) ImGui::SetItemDefaultFocus( );
                } );
                ImGui::EndCombo( );
            }

            if ( !m_backups.empty( ) ) {
                if ( ImGui::BeginListBox( "##backups", ImVec2( -FLT_MIN, content_height ) ) ) {
                    utils::enumerate( m_backups, [&]( int gi, auto& path ) {
                        if ( path.filename( ) == "undo.zip" ) {
                            return;
                        }
                        std::string label =
                            std::format( "{}##{}", path.filename( ).string( ), static_cast<int>( gi ) );
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
    }

    ImGui::SameLine( 0.0f, 10.0f );

    {
        ChildGuard remote( "##transfer_remote", ImVec2( half, bottom_height ), ImGuiChildFlags_Borders );
        ImGui::PushFont( CFontManager::get( ).get_font( "jbm_med" ).value_or( nullptr ) );
        ImGui::Text( "Remote" );
        ImGui::PopFont( );

        if ( !m_connected ) {
            ImGui::TextDisabled( "Connect to browse remote server" );
        } else {
            bool has_remote_selection =
                m_selected_remote_idx >= 0 && m_selected_remote_idx < (int)m_remote_entries.size( );
            bool is_file_selected = has_remote_selection && !m_remote_entries[m_selected_remote_idx].is_directory;

            ImGui::BeginDisabled( !is_file_selected || is_transferring );
            if ( ImGui::Button( "Download" ) ) {
                if ( m_current_remote_path.empty( ) ) {
                    Notify::show_notification(
                        "Download Failure", "Failed to download file: No remote path specified!", 2000 );
                } else {
                    std::string path = m_current_remote_path + ( m_current_remote_path.back( ) == '/' ? "" : "/" ) +
                                       m_remote_entries[m_selected_remote_idx].name;

                    m_downloading = true;
                    m_queue.run<int>(
                        [remote = m_remote, path]( TaskControl& ) {
                            if ( !remote->download_file( path, CConfig::get( ) ) ) {
                                auto str = std::format( "Failed to download: {}", path );
                                Notify::show_notification( "Download", str, 2500 );
                            } else {
                                auto str = std::format( "Downloaded {}!", path );
                                Notify::show_notification( "Download", str, 2500 );
                            }
                            return 0;
                        },
                        [this]( int ) { m_downloading = false; },
                        [this]( const std::exception& ex ) {
                            m_downloading = false;
                            Notify::show_notification( "Download", ex.what( ), 2500 );
                        } );
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

                utils::enumerate( m_remote_entries, [&]( int gi, auto& entry ) {
                    if ( entry.name == "." || entry.name == ".." ) {
                        return;
                    }

                    std::string prefix = entry.is_directory ? "[DIR] " : "[FILE] ";
                    std::string label = std::format( "{}{}##{}", prefix, entry.name, static_cast<int>( gi ) );
                    if ( is_transferring ) ImGui::BeginDisabled( true );
                    if ( ImGui::Selectable(
                             label.c_str( ), m_selected_remote_idx == static_cast<int>( gi ),
                             ImGuiSelectableFlags_AllowDoubleClick ) ) {
                        if ( ImGui::IsMouseDoubleClicked( 0 ) && entry.is_directory ) {
                            if ( m_current_remote_path.empty( ) ) {
                                Notify::show_notification(
                                    "Download failure", "Failed to download file: No remote path specified!", 2000 );
                                return;
                            }
                            m_current_remote_path = m_current_remote_path +
                                                    ( m_current_remote_path.back( ) == '/' ? "" : "/" ) + entry.name;
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
    }
}

CTransferView::~CTransferView( ) { m_queue.shutdown( ); }
