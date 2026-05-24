#include "app.hpp"
#include <constants.hpp>

#include <frontend/fonts/font_awesome.hpp>
#include <frontend/fonts/jbm_reg.h>
#include <frontend/theme/theme.hpp>

#include <frontend/views/dashboard/dashboard_view.hpp>
#include <frontend/views/debug/debug_view.hpp>
#include <frontend/views/pipeline/pipeline_demo.hpp>
#include <frontend/views/settings/settings_view.hpp>

#include <frontend/dialogs/confirm/confirm_dialog.hpp>
#include <frontend/notification/notification.hpp>

void CApp::init( ) {
    ThemeManager::apply_style( );

    m_ui_manager.add_view( { std::make_unique<CDashboardView>( ), "\xef\x80\x95", "Dashboard" } );
    m_ui_manager.add_view( { std::make_unique<CPipelineView>( ), "\xef\x83\xa8", "Pipeline Demo" } );
    m_ui_manager.add_view( { std::make_unique<CDebugView>( ), "\xef\x86\x88", "Debug" } );
    m_ui_manager.set_settings_view( { std::make_unique<CSettingsView>( m_config ), "\xef\x80\x93", "Settings" } );
}

void CApp::render( ) {
    ThemeManager::apply_colors( m_config.settings.dark_mode ? ThemeType::Dark : ThemeType::Light );
    m_ui_manager.render( );
    Notify::render_notifications( );
    ConfirmDialog::render( );
}
