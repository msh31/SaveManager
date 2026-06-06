#include "settings.hpp"
#include "config/config.hpp"
#include "frontend/ui/notifications/notification.hpp"
#include "network/network.hpp"
#include "utils/blacklist/blacklist.hpp"
#include "utils/paths.hpp"

#ifdef __APPLE__
#include <spawn.h>
#include <sys/wait.h>
#endif

void SettingsTab::render( const Fonts &fonts, Config &config ) {
    spinner_frame++;

    if ( update_future.valid( ) && update_future.wait_for( std::chrono::seconds( 0 ) ) == std::future_status::ready ) {
        bool result = update_future.get( );

        if ( result ) {
            Notify::show_notification( "Update Available!", "A new release is available for download!", 2500 );
        } else {
            Notify::show_notification( "Updates", "No new updates found!", 2500 );
        }
    }

    if ( update_t_future.valid( ) &&
         update_t_future.wait_for( std::chrono::seconds( 0 ) ) == std::future_status::ready ) {
        auto [ubi, steam] = update_t_future.get( );

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
        update_future.valid( ) && update_future.wait_for( std::chrono::seconds( 0 ) ) != std::future_status::ready;
    bool is_checking_t =
        update_t_future.valid( ) && update_t_future.wait_for( std::chrono::seconds( 0 ) ) != std::future_status::ready;

    float half = ( ImGui::GetWindowSize( ).x - 20.0f ) / 2.0f;
    auto window_width = ( ImGui::GetWindowSize( ).x / 3.0f );

    ImGui::PushFont( fonts.header );
    ImGui::Text( "Settings" );
    ImGui::PopFont( );

    ImGui::BeginChild( "##appearance", ImVec2( window_width, 250.0f ), true,
                       ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse );

    ImGui::PushFont( fonts.medium );
    ImGui::Text( "Appearance" );
    ImGui::PopFont( );

    ImGui::Dummy( ImVec2( 0.0f, 4.0f ) );
    ImGui::Checkbox( "Dark Mode", &config.settings.dark_mode );
    ImGui::SameLine( );
    ImGui::Checkbox( "Animated background", &config.settings.animated_background );
    ImGui::Separator( );

    if ( is_checking ) ImGui::BeginDisabled( true );
    if ( ImGui::Button( "Check for updates" ) ) {
        update_future = std::async( std::launch::async, []( ) { return Network::is_update_available( ); } );
    }
    if ( is_checking ) ImGui::EndDisabled( );
    ImGui::SameLine( );
    if ( is_checking_t ) ImGui::BeginDisabled( true );
    if ( ImGui::Button( "Update translations" ) ) {
        update_t_future = std::async( std::launch::async, [this]( ) -> std::pair<bool, bool> {
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

    ImGui::BeginChild( "##support", ImVec2( window_width, 250.0f ), true,
                       ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse );

    ImGui::PushFont( fonts.medium );
    ImGui::Text( "Launcher Support" );
    ImGui::PopFont( );

    ImGui::Checkbox( "Ubisoft Connect", &config.settings.ubi_enabled );
    ImGui::Checkbox( "Rockstar Games Launcher", &config.settings.rsg_enabled );
    ImGui::Checkbox( "Unreal Games (.sav saves)", &config.settings.unreal_enabled );
    ImGui::EndChild( );

    ImGui::SameLine( 0.0f, 10.0f );

    ImGui::BeginChild( "##blacklisted_games", ImVec2( 0, 250.0f ), true,
                       ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse );

    ImGui::PushFont( fonts.medium );
    ImGui::Text( "Blacklisted Games" );
    ImGui::PopFont( );

    if ( ImGui::BeginChild( "blacklist_child", ImVec2( 0, 120 ), true ) ) {
        int i = 0;
        for ( auto it = Blacklist::blacklisted_games.begin( ); it != Blacklist::blacklisted_games.end( ); ) {
            ImGui::Text( "%s", it->c_str( ) );
            ImGui::SameLine( );
            ImGui::SetCursorPosX( ImGui::GetCursorPosX( ) + 5 );
            if ( ImGui::Button( std::format( "X##{}", i ).c_str( ) ) ) {
                it = Blacklist::blacklisted_games.erase( it );
                Blacklist::save( );
            } else {
                ++it;
                ++i;
            }
        }
        ImGui::EndChild( );
    } else {
        ImGui::EndChild( );
    }

    ImGui::InputText( "##blacklist_input", &blacklist_input );
    ImGui::SameLine( );
    if ( ImGui::Button( "Add##blacklist" ) ) {
        if ( !blacklist_input.empty( ) ) {
            Blacklist::blacklisted_games.insert( blacklist_input );
            Blacklist::save( );
            blacklist_input.clear( );
        }
    }
    ImGui::EndChild( );
}
