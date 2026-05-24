#include "app.hpp"
#include <constants.hpp>

#include <frontend/fonts/font_awesome.hpp>
#include <frontend/fonts/jbm_reg.h>
#include <frontend/theme/theme.hpp>

#include <frontend/views/cache_demo/cache_demo_view.hpp>
#include <frontend/views/debug/debug_view.hpp>
#include <frontend/views/home/home_view.hpp>
#include <frontend/views/pipeline/pipeline_demo.hpp>
#include <frontend/views/settings/settings_view.hpp>

#include <frontend/dialogs/confirm/confirm_dialog.hpp>
#include <frontend/notification/notification.hpp>

void CApp::init( ) {
    ThemeManager::apply_style( );

    m_ui_manager.add_view( { std::make_unique<CHomeView>( ), "\xef\x80\x95", "Home" } );
    m_ui_manager.add_view( { std::make_unique<CPipelineView>( ), "\xef\x83\xa8", "Pipeline Demo" } );
    m_ui_manager.add_view( { std::make_unique<CDebugView>( ), "\xef\x86\x88", "Debug" } );
    m_ui_manager.add_view( { std::make_unique<CCacheDemoView>( ), "\xef\xbb\x9b", "Cache Demo" } );
    m_ui_manager.set_settings_view( { std::make_unique<CSettingsView>( m_config ), "\xef\x80\x93", "Settings" } );

    m_menubar.add_group( { "File",
                           {
                               { "\xef\x80\x81", "New", [] { Notify::show_notification( "File", "New", 1500 ); } },
                               { "\xef\x81\xbb", "Open", [] { Notify::show_notification( "File", "Open", 1500 ); } },
                               { "\xef\x83\x87", "Save", [] { Notify::show_notification( "File", "Saved!", 1500 ); } },
                           } } );
    m_menubar.add_group( { "Options",
                           {
                               { "\xef\x80\x93", "Dark Mode", nullptr, &m_config.settings.dark_mode },
                               { "\xef\x83\xa8", "Feature A", nullptr, &m_toggle_a },
                               { "\xef\x86\x88", "Feature B", nullptr, &m_toggle_b },
                               { "\xef\x80\x95", "Feature C", nullptr, &m_toggle_c },
                               { "\xef\x80\x81", "Feature D", nullptr, &m_toggle_d },
                               { "\xef\x81\xbb", "Feature E", nullptr, &m_toggle_e },
                           } } );
    m_ui_manager.set_menubar( std::move( m_menubar ) );

    m_statusbar.add_left( { "I am a statusbar", "X" } );
    m_statusbar.add_right( { "Build", "1.69" } );
    m_ui_manager.set_statusbar( std::move( m_statusbar ) );
}

void CApp::render( ) {
    ThemeManager::apply_colors( m_config.settings.dark_mode ? ThemeType::Dark : ThemeType::Light );
    m_ui_manager.render( );
    Notify::render_notifications( );
    ConfirmDialog::render( );
}
