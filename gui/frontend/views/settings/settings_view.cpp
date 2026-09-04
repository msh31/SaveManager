#include "settings_view.hpp"
#include <utils/image_extensions.hpp>
#include <utils/utils.hpp>
#include <config/config.hpp>
#include <detection/detection_service.hpp>
#include <utils/network.hpp>

#include <backend/font_manager/font_manager.hpp>

#include <frontend/childguard.hpp>
#include <frontend/notification/notification.hpp>
#include <frontend/theme/theme.hpp>

void CSettingsView::on_enter( ) {
    m_backgrounds.clear( );
    
    //unlikely to happen but guard it anyway
    if ( fs::exists( paths::backgrounds_dir( ) ) ) {
        for ( const auto& f :
              fs::directory_iterator( paths::backgrounds_dir( ), fs::directory_options::skip_permission_denied ) ) {
            if ( f.is_regular_file( ) && utils::is_image_file( f.path( ) ) ) {
                m_backgrounds.emplace_back( f.path( ).filename( ).string( ) );
            }
        }
    
        m_current_background = 0;
        auto it = std::find( m_backgrounds.begin( ), m_backgrounds.end( ), CConfig::get().settings.bg_name );
        if ( it != m_backgrounds.end( ) ) {
            m_current_background = (int)std::distance( m_backgrounds.begin( ), it );
        }
    }
}

void CSettingsView::render( ) {
    m_queue.update( );

    {
        ChildGuard appearance( "Appearance", { 300.0f, 0.0f } );
        ImGui::Text( "Appearance" );

        if ( ImGui::Checkbox( "Dark Mode", &CConfig::get().settings.dark_mode ) ) {
            ThemeManager::apply_colors( CConfig::get().settings.dark_mode ? ThemeType::Dark : ThemeType::Light );
        }

        ImGui::Separator( );

        ImGui::Checkbox( "Use Shader", &CConfig::get( ).settings.animated_background );

        // ImGui::Separator( );
        // 
        //if ( (int)m_backgrounds.size( ) <= 0 ) {
        //    auto str = std::format( "No backgrounds found, add some here: {}", paths::backgrounds_dir( ).string( ) );
        //    ImGui::TextWrapped( "%s", str.c_str( ) );
        //} else {
        //    ImGui::Checkbox( "Custom Background", &CConfig::get().settings.use_bg );

        //    if ( ImGui::BeginListBox( "##bg_list" ) ) {
        //        for ( int i = 0; i < (int)m_backgrounds.size( ); i++ ) {
        //            bool is_selected = ( m_current_background == i );
        //            if ( ImGui::Selectable( m_backgrounds[i].c_str( ), is_selected ) ) {
        //                m_current_background = i;
        //                CConfig::get().settings.bg_name = m_backgrounds[i];
        //            }
        //        }
        //        ImGui::EndListBox( );
        //    }
        //}
    }

    ImGui::SameLine( );

    {
        ChildGuard config( "Configuration", { 0.0f, 0.0f } );
        ImGui::Text( "Configuration" );

        ImGui::Checkbox( "Check for updates on startup", &CConfig::get( ).settings.startup_update_check );
        ImGui::Separator( );

        if ( ImGui::Button( "Open" ) ) {
            utils::open_in_file_manager( paths::config_dir( ).string( ).c_str( ) );
        }
        ImGui::SetItemTooltip( "Opens your file manager to the config directory." );

        ImGui::SameLine( );
        if ( ImGui::Button( "Save" ) ) {
            CConfig::get( ).save( );
            Notify::show_notification( "Config", "Saved config!", 1500 );
        }

        ImGui::Separator( );

        if ( m_update_handle.has_value( ) ) ImGui::BeginDisabled( true );
        if ( ImGui::Button( "Check for updates" ) ) {
            m_update_handle = m_queue.run<bool>(
                []( TaskControl& control ) {
                    if ( control.cancel_requested.load( ) ) throw TaskCancelled{ };
                    return Network::is_update_available( );
                },
                [this]( bool nva ) {
                    if (nva) {
                        Notify::show_notification( "Update Check", "A new update is available to download!", 1500 );
                    }
                    else {
                        Notify::show_notification( "Update Check", "No updates found!", 1500 );
                    }
                    m_update_handle = std::nullopt;
                },
                []( const std::exception& ex ) { Notify::show_notification( "Error", ex.what( ), 5000 ); } );
        }
        if ( m_update_handle.has_value( ) ) ImGui::EndDisabled( );
        ImGui::SameLine( );
        if ( m_ubi_translations_handle.has_value( ) || m_manifest_handle.has_value( ) ) ImGui::BeginDisabled( true );
        if ( ImGui::Button( "Update translations" ) ) {
            m_ubi_translations_handle = m_queue.run<bool>(
            []( TaskControl& control ) {
                if ( control.cancel_requested.load( ) ) throw TaskCancelled{ };
                    return Network::download_file( ubi_translation_url.data( ), paths::ubi_translations( ).string( ) );
            },
            [this]( bool nva ) {
                if ( nva ) {
                    Notify::show_notification( "Update Check", "Updated ubisoft translations!", 1500 );
                }
                m_ubi_translations_handle = std::nullopt;
            },
            []( const std::exception& ex ) { Notify::show_notification( "Error", ex.what( ), 5000 ); } );

            m_manifest_handle = m_queue.run<bool>(
            []( TaskControl& control ) {
                if ( control.cancel_requested.load( ) ) throw TaskCancelled{ };
                    return Network::download_file( pcgw_translation_url.data( ), paths::pcgw_manifest( ).string( ) );
            },
            [this]( bool nva ) {
                if ( nva ) {
                    Notify::show_notification( "Update Check", "Updated manifest!", 1500 );
                }
                m_manifest_handle = std::nullopt;
            },
            []( const std::exception& ex ) { Notify::show_notification( "Error", ex.what( ), 5000 ); } );
        }
        ImGui::SetItemTooltip( "Forces a new download of the ubisoft id translations" );
        if ( m_ubi_translations_handle.has_value( ) || m_manifest_handle.has_value( ) ) ImGui::EndDisabled( );
    }
    
    ImGui::SameLine( );

    {
        ChildGuard detection( "Detection", { } );
        ImGui::PushFont( CFontManager::get( ).get_font( "jbm_med" ).value_or( nullptr ) );
        ImGui::Text( "Detection" );
        ImGui::PopFont( );

        ImGui::Checkbox( "Show conflicting files", &CConfig::get( ).d_settings.show_conflicts );
        ImGui::Checkbox( "Use ignore files", &CConfig::get( ).d_settings.use_savemgr_ignore );
        ImGui::Checkbox( "Skip empty files", &CConfig::get( ).d_settings.skip_empty_files );
    }

    
    {
        ChildGuard blacklist( "Blacklist", { } );
        ImGui::PushFont( CFontManager::get( ).get_font( "jbm_med" ).value_or( nullptr ) );
        ImGui::Text( "Blacklist" );
        ImGui::PopFont( );

        {
            ChildGuard blacklist_entries( "blacklist_entries", { } );

            int i = 0;
            std::string game_to_remove = { };
            auto games = CDetectionService::get( ).blacklist( ).games( );

            for ( auto it = games.begin( ); it != games.end( ); ++it, ++i ) {
                ImGui::Text( "%s", it->c_str( ) );
                ImGui::SameLine( );
                ImGui::SetCursorPosX( ImGui::GetCursorPosX( ) + 5 );
                if ( ImGui::Button( std::format( "X##{}", i ).c_str( ) ) ) {
                    game_to_remove = *it;
                }
            }

            if ( !game_to_remove.empty( ) ) {
                CDetectionService::get( ).blacklist( ).remove( game_to_remove );
            }
        }

        ImGui::InputText( "##m_blacklist_input", &m_blacklist_input );
        //ImGui::SameLine( );
        if ( ImGui::Button( "Add##blacklist" ) ) {
            if ( !m_blacklist_input.empty( ) ) {
                CDetectionService::get( ).blacklist( ).add( m_blacklist_input );
                m_blacklist_input.clear( );
            }
        }
    }
}

void CSettingsView::on_exit( ) { m_backgrounds.clear( ); }

CSettingsView::~CSettingsView( ) { m_queue.shutdown( ); }
