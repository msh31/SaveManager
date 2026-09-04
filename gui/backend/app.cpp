#include "app.hpp"
#include <branding.hpp>
#include <config/config.hpp>
#include <logger.hpp>
#include <utils/network.hpp>
#include <detection/detection_service.hpp>

#include <frontend/icons.hpp>
#include <frontend/theme/theme.hpp>
#include <frontend/ui.hpp>

#include <frontend/views/home/home_view.hpp>
#include <frontend/views/log/log_view.hpp>
#include <frontend/views/settings/settings_view.hpp>

#include <frontend/components/dialogs/confirm/confirm_dialog.hpp>
#include <frontend/notification/notification.hpp>

void CApp::init( ) {
    m_update_handle = m_queue.run<bool>(
        []( TaskControl& control ) {
            if ( control.cancel_requested.load( ) ) throw TaskCancelled{ };
            return Network::is_update_available( );
        },
        [this]( bool nva ) {
            if ( nva ) Notify::show_notification( "Update Check", "A new update is available to download!", 1500 );
            m_update_handle = std::nullopt;
        },
        []( const std::exception& ex ) { Notify::show_notification( "Error", ex.what( ), 5000 ); 
    } );

    //init - cant be cnaceled
    m_detection_handle = m_queue.run<std::monostate>(
        [this]( TaskControl& control ) {
            // kinda odd
            CDetectionService::get( ).init( );
            return std::monostate{ };
        },
        [this]( std::monostate ) {
            CDetectionService::get( ).ensure_started( );
            m_detection_handle = std::nullopt;
        },
        []( const std::exception& ex ) { Notify::show_notification( "Detection init error", ex.what( ), 5000 ); 
    } );

    refresh_background( );

    SPDLOG_INFO( "Setting up application views.." );
    m_ui_manager.add_view( { std::make_unique<CHomeView>( ), ICON_HOME, "Home" } );
    m_ui_manager.add_view( { std::make_unique<CLogView>( ), ICON_SCROLL, "Logs" } );
    m_ui_manager.set_settings_view( { std::make_unique<CSettingsView>( ), ICON_GEAR, "Settings" } );
}

void CApp::refresh_background( ) {
    if ( !CConfig::get( ).settings.use_bg || CConfig::get( ).settings.bg_name.empty( ) ) {
        m_loaded_bg_name.clear( );
        return;
    }

    if ( CConfig::get( ).settings.bg_name == m_loaded_bg_name ) return;

    auto path = paths::backgrounds_dir( ) / CConfig::get( ).settings.bg_name;
    if ( fs::exists( path ) ) {
        m_background_image = CImageManager::get( ).load_from_disk( path, "background" );
        m_loaded_bg_name = CConfig::get( ).settings.bg_name;
    } else {
        CConfig::get( ).settings.use_bg = false;
        Notify::show_notification(
            "Custom Background", "Failed to apply custom background because the file does not exist!", 1500 );
    }
}

void CApp::render( ) {
    refresh_background( );
    m_queue.update( );

    bool use_bg = CConfig::get( ).settings.use_bg;

    if ( use_bg ) {
        ui::add_cover_image(
            ImGui::GetBackgroundDrawList( ), (ImTextureID)m_background_image.texture_id, ImVec2( 0, 0 ),
            ImGui::GetIO( ).DisplaySize, (float)m_background_image.texture_width,
            (float)m_background_image.texture_height );
        ImGui::PushStyleColor( ImGuiCol_ChildBg, ImVec4( 0, 0, 0, 0 ) );
    }

    m_ui_manager.render( );

    if ( use_bg ) {
        ImGui::PopStyleColor( );
    }

    // rendered on top of the general UI
    Notify::render_notifications( );
    ConfirmDialog::render( );
}

void CApp::on_files_dropped(const std::vector<std::string>& files) {
    //m_debug_view->set_dropped_paths(files);
}