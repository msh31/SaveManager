#include "app.hpp"
#include <constants.hpp>

#include <utils/steam/steam.hpp>

#include <frontend/fonts/font_awesome.hpp>
#include <frontend/fonts/jbm_reg.h>
#include <frontend/icons.hpp>
#include <frontend/theme/theme.hpp>

#include <frontend/views/about/about_view.hpp>
#include <frontend/views/dashboard/dashboard_view.hpp>
#include <frontend/views/editor/editor_view.hpp>
#include <frontend/views/log/log_view.hpp>
#include <frontend/views/settings/settings_view.hpp>
#include <frontend/views/transfer/transfer_view.hpp>

#include <frontend/dialogs/confirm/confirm_dialog.hpp>
#include <frontend/notification/notification.hpp>

#include <features/features.hpp> //sus - for label migration

CApp::~CApp( ) {
    SPDLOG_INFO( "Exiting SaveManager.." );
    m_task_runner.shutdown( );
}

void CApp::init( ) {
    ThemeManager::apply_style( );
    if ( !m_config.init( ) ) {
        throw std::runtime_error( "Config is missing and could not be generated!" );
    }
    if ( !m_translations.init( ) ) {
        SPDLOG_WARN( "Failed to initialize translations! Expect missing games!" );
    }
    if ( !m_blacklist.init( ) ) {
        SPDLOG_WARN( "Failed to initialize blacklist!" );
    }
    if ( !m_manifest_cache.init( ) ) {
        SPDLOG_WARN( "Failed to initialize Steam manifest cache! Expect missing Unreal game names!" );
    }
    if ( !m_unreal_name_cache.init( ) ) {
        SPDLOG_WARN( "Failed to load cached Unreal game names!" );
    }

    // TODO: remove this in the future
    Features::migrate_labels_to_tags( );

    m_ui_manager.add_view(
        { std::make_unique<CDashboardView>( m_config, m_detection ), ICON_HOME, "Dashboard" } );
    m_ui_manager.add_view( { std::make_unique<CEditorView>( ), ICON_EDIT, "Save Editor" } );
    m_ui_manager.add_view(
        { std::make_unique<CTransferView>( m_config, m_detection ), ICON_TRANSFER, "Transfer" } );
    m_ui_manager.add_view( { std::make_unique<CAboutView>( ), ICON_INFO, "About" } );
    m_ui_manager.add_view( { std::make_unique<CLogView>( ), ICON_SCROLL, "Log" } );
    m_ui_manager.set_settings_view(
        { std::make_unique<CSettingsView>( m_config, m_blacklist ), ICON_GEAR, "Settings" } );
    m_shader.emplace( );
}

void CApp::render( ) {
    m_task_runner.update( );
    ThemeManager::apply_colors( m_config.settings.dark_mode ? ThemeType::Dark : ThemeType::Light );
    m_ui_manager.render( );
    Notify::render_notifications( );
    ConfirmDialog::render( );
}

void CApp::save_window_props( int x, int y, int width, int height ) {
    m_config.win_props = { x, y, width, height };
    m_config.save( );
}

// called from main before render so the shader actually displays
void CApp::render_shader( std::pair<int, int> window_size ) {
    if ( m_config.settings.dark_mode ) {
        glClearColor( 0.145f, 0.145f, 0.141f, 1.0f );
    } else {
        glClearColor( 0.980f, 0.976f, 0.961f, 1.00f );
    }
    glClear( GL_COLOR_BUFFER_BIT ); // for the themes
    glViewport( 0, 0, window_size.first, window_size.second );

    if ( m_config.settings.animated_background && m_shader.has_value( ) ) {
        m_shader->render( window_size.first, window_size.second );
    }
}
