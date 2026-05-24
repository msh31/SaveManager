#include "settings_view.hpp"

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
    ImGui::Checkbox( "Dark Mode", &m_config.settings.dark_mode );

    if ( ImGui::Button( "Open config" ) ) {
#ifdef __linux__
        pid_t pid = fork( );
        if ( pid == 0 ) {
            execl( "/usr/bin/xdg-open", "xdg-open", paths::config_dir( ).string( ).c_str( ), nullptr );
            _exit( 1 );
        }
#endif
#ifdef _WIN32
        ShellExecuteA( NULL, "open", paths::config_dir( ).string( ).c_str( ), NULL, NULL, SW_SHOWDEFAULT );
#endif
#ifdef __APPLE__
        extern char** environ;
        pid_t         pid;
        std::string   path = paths::config_dir( ).string( );

        const char* argv[] = { "open", path.c_str( ), nullptr };
        int         status = posix_spawn( &pid, "/usr/bin/open", nullptr, nullptr, (char* const*)argv, environ );
        if ( status == 0 ) {
            waitpid( pid, &status, 0 );
        }
#endif
    }
    ImGui::SetItemTooltip( "Opens your file manager to the config directory." );
}

void CSettingsView::on_exit( ) {}

CSettingsView::~CSettingsView( ) { SPDLOG_INFO( "goodbye: settingsview" ); }
