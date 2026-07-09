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
        auto [ubi, steam] = m_update_t_future.get( );

        if ( !ubi ) {
            SPDLOG_ERROR( "Failed to download Ubisoft translations" );
            Notify::show_notification( "Translations", "Failed to update translations for ubisoft", 2500 );
        }
        if ( !steam ) {
            SPDLOG_ERROR( "Failed to download Steam ID data" );
            Notify::show_notification( "Translations", "Failed to update translations for steam appids", 2500 );
        }
        if ( steam && ubi ) {
            Notify::show_notification( "Translations", "All translations have been updated!", 2500 );
        }
    }

    bool is_checking =
        m_update_future.valid( ) && m_update_future.wait_for( std::chrono::seconds( 0 ) ) != std::future_status::ready;
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
    ImGui::Text( "Appearance" );
    ImGui::PopFont( );

    ImGui::Dummy( ImVec2( 0.0f, 4.0f ) );
    ImGui::Checkbox( "Dark Mode", &m_config.settings.dark_mode );
    ImGui::SameLine( );
    ImGui::Checkbox( "Animated background", &m_config.settings.animated_background );
    ImGui::Separator( );

    if ( is_checking ) ImGui::BeginDisabled( true );
    if ( ImGui::Button( "Check for updates" ) ) {
        m_update_future = std::async( std::launch::async, []( ) { return Network::is_update_available( ); } );
    }
    if ( is_checking ) ImGui::EndDisabled( );
    ImGui::SameLine( );
    if ( is_checking_t ) ImGui::BeginDisabled( true );
    if ( ImGui::Button( "Update translations" ) ) {
        m_update_t_future = std::async( std::launch::async, [this]( ) -> std::pair<bool, bool> {
            bool ubi = Network::download_file( ubi_translation_url, paths::ubi_translations( ).string( ) );
            bool steam = Network::download_file( steam_translation_url, paths::steam_appids( ).string( ) );
            return { ubi, steam };
        } );
    }
    ImGui::SetItemTooltip( "Forces a new download of the ubisoft id and steam id translations" );
    if ( is_checking_t ) ImGui::EndDisabled( );
    if ( ImGui::Button( "Open config" ) ) {
        open_in_file_manager( paths::config_dir( ).string( ).c_str( ) );
    }
    ImGui::SetItemTooltip( "Opens your file manager to the config directory." );
    ImGui::EndChild( );

    ImGui::SameLine( 0.0f, 10.0f );

    ImGui::BeginChild(
        "##blacklisted_games", ImVec2( 0, 250.0f ), true,
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
        m_blacklist.remove( game_to_remove );
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
}

void CSettingsView::on_exit( ) {}

CSettingsView::~CSettingsView( ) {}
