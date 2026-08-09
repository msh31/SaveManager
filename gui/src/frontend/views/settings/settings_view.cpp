#include "settings_view.hpp"

#include <backend/font_manager/font_manager.hpp>
#include <frontend/notification/notification.hpp>

#include <network/network.hpp>

#include <utils/utils.hpp>

void CSettingsView::on_enter( ) {}

void CSettingsView::render( ) {
    if ( m_update_future.valid( ) &&
         m_update_future.wait_for( std::chrono::seconds( 0 ) ) == std::future_status::ready ) {
        bool result = m_update_future.get( );

        if ( result ) {
            Notify::show_notification( "Update Available!", "A new release is available for download!", 2500 );
        } else {
            Notify::show_notification( "Updates", "No new updates found!", 2500 );
        }
    }

    if ( m_update_t_future.valid( ) &&
         m_update_t_future.wait_for( std::chrono::seconds( 0 ) ) == std::future_status::ready ) {
       bool res = m_update_t_future.get( );

        if ( !res ) {
            SPDLOG_ERROR( "Failed to download Ubisoft translations" );
            Notify::show_notification( "Translations", "Failed to update translations for ubisoft", 2500 );
        }
        else {
            Notify::show_notification( "Translations", "Updated translations successfully!", 2500 );
        }
    }

    bool is_checking =
        m_update_future.valid( ) && m_update_future.wait_for( std::chrono::seconds( 0 ) ) != std::future_status::ready;

    // translations..
    bool is_checking_t = m_update_t_future.valid( ) &&
                         m_update_t_future.wait_for( std::chrono::seconds( 0 ) ) != std::future_status::ready;

    float half = ( ImGui::GetWindowSize( ).x - 20.0f ) / 2.0f;
    auto window_width = ( ImGui::GetWindowSize( ).x / 3.0f );

    ImGui::PushFont( CFontManager::get( ).get_font( "jbm_header" ).value_or( nullptr ) );
    ImGui::Text( "Settings" );
    ImGui::PopFont( );

    ImGui::BeginChild(
        "##appearance", ImVec2( window_width, 250.0f ), true,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse );

    ImGui::PushFont( CFontManager::get( ).get_font( "jbm_med" ).value_or( nullptr ) );
    ImGui::Text( "General" );
    ImGui::PopFont( );

    ImGui::Dummy( ImVec2( 0.0f, 4.0f ) );
    ImGui::Checkbox( "Dark Mode", &m_config.settings.dark_mode );
    ImGui::SameLine( );
    ImGui::Checkbox( "Animated background", &m_config.settings.animated_background );
    ImGui::Separator( );
    ImGui::Checkbox( "Check for updates on startup", &m_config.settings.startup_update_check );

    if ( is_checking ) ImGui::BeginDisabled( true );
    if ( ImGui::Button( "Check for updates" ) ) {
        m_update_future = std::async( std::launch::async, []( ) { return Network::is_update_available( ); } );
    }
    if ( is_checking ) ImGui::EndDisabled( );
     ImGui::SameLine( );
     if ( is_checking_t ) ImGui::BeginDisabled( true );
     if ( ImGui::Button( "Update translations" ) ) {
         m_update_t_future = std::async( std::launch::async, [this]( ) -> bool {
             return Network::download_file( ubi_translation_url, paths::ubi_translations( ).string( ) );
         } );
     }
     ImGui::SetItemTooltip( "Forces a new download of the ubisoft id translations" );
     if ( is_checking_t ) ImGui::EndDisabled( );
    if ( ImGui::Button( "Open config" ) ) {
        open_in_file_manager( paths::config_dir( ).string( ).c_str( ) );
    }
    ImGui::SetItemTooltip( "Opens your file manager to the config directory." );
    ImGui::SameLine( );
    if ( ImGui::Button( "Save" ) ) {
        m_config.save( );
        Notify::show_notification( "Config", "Saved app settings!", 2000 );
    }
    ImGui::EndChild( );

    ImGui::SameLine( 0.0f, 10.0f );

    ImGui::BeginChild(
        "##blacklisted_games", ImVec2( window_width, 250.0f ), true,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse );

    ImGui::PushFont( CFontManager::get( ).get_font( "jbm_med" ).value_or( nullptr ) );
    ImGui::Text( "Blacklisted Games" );
    ImGui::PopFont( );

    if ( ImGui::BeginChild( "blacklist_child", ImVec2( 0, 120 ), true ) ) {
        int i = 0;
        std::string game_to_remove = { };
        auto games = m_blacklist.games( );
        for ( auto it = games.begin( ); it != games.end( ); ++it, ++i ) {
            ImGui::Text( "%s", it->c_str( ) );
            ImGui::SameLine( );
            ImGui::SetCursorPosX( ImGui::GetCursorPosX( ) + 5 );
            if ( ImGui::Button( std::format( "X##{}", i ).c_str( ) ) ) {
                game_to_remove = *it;
            }
        }
        if ( !game_to_remove.empty( ) ) {
            m_blacklist.remove( game_to_remove );
        }
        ImGui::EndChild( );
    } else {
        ImGui::EndChild( );
    }

    ImGui::InputText( "##m_blacklist_input", &m_blacklist_input );
    ImGui::SameLine( );
    if ( ImGui::Button( "Add##blacklist" ) ) {
        if ( !m_blacklist_input.empty( ) ) {
            m_blacklist.add( m_blacklist_input );
            m_blacklist_input.clear( );
        }
    }
    ImGui::EndChild( );

    ImGui::SameLine( 0.0f, 10.0f );

    ImGui::BeginChild(
        "##detection_settings", ImVec2( 350.f, 250.0f ), true, // stupid
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse );

    ImGui::PushFont( CFontManager::get( ).get_font( "jbm_med" ).value_or( nullptr ) );
    ImGui::Text( "Detection" );
    ImGui::PopFont( );

    ImGui::Checkbox( "Show conflicting files", &m_config.d_settings.show_conflicts );
    ImGui::Checkbox( "Use ignore files", &m_config.d_settings.use_savemgr_ignore );
    ImGui::Checkbox( "Skip empty files", &m_config.d_settings.skip_empty_files );

    ImGui::EndChild( );
}

void CSettingsView::on_exit( ) {}

CSettingsView::~CSettingsView( ) {}
