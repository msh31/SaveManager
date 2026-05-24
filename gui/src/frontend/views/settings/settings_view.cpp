#include "settings_view.hpp"

#include <backend/font_manager/font_manager.hpp>
#include <frontend/notification/notification.hpp>

#include <network/network.hpp>

#include <utils/blacklist/blacklist.hpp>
#include <utils/utils.hpp>

#ifdef __APPLE__
    #include <spawn.h>
    #include <sys/wait.h>
#endif
#ifdef _WIN32
    #include <shellapi.h>
#endif

CSettingsView::CSettingsView( CConfig& cfg ) : m_config( cfg ) {};

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

    float half         = ( ImGui::GetWindowSize( ).x - 20.0f ) / 2.0f;
    auto  window_width = ( ImGui::GetWindowSize( ).x / 3.0f );

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

    if ( !is_checking ) {
        if ( ImGui::Button( "Check for updates" ) ) {
            m_update_future = std::async( std::launch::async, []( ) { return Network::is_update_available( ); } );
        }
    } else {
        // char spin_char = spinner[( spinner_frame / 10 ) % 4];
        // ImGui::Text( "%s", std::format( "Checking for updates {}", spin_char ).c_str( ) );
    }
    ImGui::SameLine( );
    if ( !is_checking_t ) {
        if ( ImGui::Button( "Update translations" ) ) {
            m_update_t_future = std::async( std::launch::async, [this]( ) -> std::pair<bool, bool> {
                bool ubi   = Network::download_file( ubi_translation_url, paths::ubi_translations( ).string( ) );
                bool steam = Network::download_file( steam_translation_url, paths::steam_appids( ).string( ) );
                return { ubi, steam };
            } );
        }
        ImGui::SetItemTooltip( "Forces a new download of the ubisoft id and steam id translations" );
    } else {
        // char spin_char = spinner[( spinner_frame / 10 ) % 4];
        // ImGui::Text( "%s", std::format( "Updating translations {}", spin_char ).c_str( ) );
    }
    if ( ImGui::Button( "Open config" ) ) {
        open_in_file_manager( paths::config_dir( ).string( ).c_str( ) );
    }
    ImGui::SetItemTooltip( "Opens your file manager to the config directory." );
    ImGui::EndChild( );
    ImGui::SameLine( 0.0f, 10.0f );

    ImGui::BeginChild(
        "##support", ImVec2( window_width, 250.0f ), true,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse );

    ImGui::PushFont( CFontManager::get( ).get_font( "jbm_med" ).value_or( nullptr ) );
    ImGui::Text( "Launcher Support" );
    ImGui::PopFont( );

    ImGui::Checkbox( "Ubisoft Connect", &m_config.settings.ubi_enabled );
    ImGui::Checkbox( "Rockstar Games Launcher", &m_config.settings.rsg_enabled );
    ImGui::Checkbox( "Unreal Games (.sav saves)", &m_config.settings.unreal_enabled );
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

    ImGui::InputText( "##m_blacklist_input", &m_blacklist_input );
    ImGui::SameLine( );
    if ( ImGui::Button( "Add##blacklist" ) ) {
        if ( !m_blacklist_input.empty( ) ) {
            Blacklist::blacklisted_games.insert( m_blacklist_input );
            Blacklist::save( );
            m_blacklist_input.clear( );
        }
    }
    ImGui::EndChild( );

    //     if ( ImGui::Button( "Open config" ) ) {
    // #ifdef __linux__
    //         pid_t pid = fork( );
    //         if ( pid == 0 ) {
    //             execl( "/usr/bin/xdg-open", "xdg-open", paths::config_dir( ).string( ).c_str( ), nullptr );
    //             _exit( 1 );
    //         }
    // #endif
    // #ifdef _WIN32
    //         ShellExecuteA( NULL, "open", paths::config_dir( ).string( ).c_str( ), NULL, NULL, SW_SHOWDEFAULT );
    // #endif
    // #ifdef __APPLE__
    //         extern char** environ;
    //         pid_t         pid;
    //         std::string   path = paths::config_dir( ).string( );
    //
    //         const char* argv[] = { "open", path.c_str( ), nullptr };
    //         int         status = posix_spawn( &pid, "/usr/bin/open", nullptr, nullptr, (char* const*)argv, environ );
    //         if ( status == 0 ) {
    //             waitpid( pid, &status, 0 );
    //         }
    // #endif
    //     }
    //     ImGui::SetItemTooltip( "Opens your file manager to the config directory." );
}

void CSettingsView::on_exit( ) {}

CSettingsView::~CSettingsView( ) { SPDLOG_INFO( "goodbye: settingsview" ); }
