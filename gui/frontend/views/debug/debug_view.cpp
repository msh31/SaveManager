#include "debug_view.hpp"
#include <nfd.h>
#include <utils/utils.hpp>

#include <frontend/childguard.hpp>

#include <frontend/dialogs/confirm/confirm_dialog.hpp>
#include <frontend/notification/notification.hpp>
#include <frontend/theme/theme.hpp>

void CDebugView::render( ) {
    m_queue.update( );

    {
        ChildGuard buttons( "buttons", { 500.0f, 0.0f }, ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY );

        if ( ImGui::Button( "Click Me" ) ) {
            Notify::show_notification( "", "Click Me button has been clicked!", 2000 );
        }

        if ( ImGui::Button( "  \xef\x80\x81  Test Icon" ) ) {
            Notify::show_notification( "", "Icon button has been clicked!", 2000 );
        }

        if ( ImGui::Button( "Test Notification" ) ) {
            Notify::show_notification( "I am a title", "I am the body", 2000 );
        }

        if ( ImGui::Button( "Test confirm dialog" ) ) {
            ConfirmDialog::show(
                "Are you sure?", [this] { Notify::show_notification( "Test", "This is a test", 1000 ); } );
        }

        if ( ImGui::Button( "Test async task" ) ) {
            m_task_handle = m_queue.run<int>(
                []( TaskControl& control ) {
                    for ( int i = 0; i < 20; i++ ) {
                        if ( control.cancel_requested.load( ) ) throw TaskCancelled{ };
                        std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
                        control.progress.store( ( i + 1 ) / 20.0f );
                    }
                    return 0;
                },
                [this]( int ) {
                    Notify::show_notification( "Async", "Task complete!", 2000 );
                    m_task_handle = std::nullopt;
                },
                []( const std::exception& ex ) { Notify::show_notification( "Error", ex.what( ), 5000 ); } );
        }

        if ( m_task_handle.has_value( ) ) {
            ImGui::SameLine( );
            bool cancelled = false;
            if ( ImGui::Button( "Cancel" ) ) {
                m_task_handle->request_cancel( );
                cancelled = true;
            }
            if ( !cancelled ) ImGui::ProgressBar( m_task_handle->progress( ) );
            if ( cancelled ) m_task_handle = std::nullopt;
        }

        ImGui::Separator( );
    }

    ImGui::SameLine( );

    {
        ChildGuard scaling( "scaling", { 200.0f, 0.0f }, ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY );
        ImGui::Text( "Scaling" );

        if ( ImGui::Button( "Set scale to 2.0x" ) ) {
            ThemeManager::apply_scale( 2.0f );
        }

        if ( ImGui::Button( "Set scale to 1.5x" ) ) {
            ThemeManager::apply_scale( 1.5f );
        }

        if ( ImGui::Button( "reset scale" ) ) {
            ThemeManager::apply_scale( 1.0f );
        }
    }

    {
        ChildGuard filepicker( "filepicker", { 500.0f, 0.0f }, ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY );
        ImGui::Text( "File Picker" );

        if ( ImGui::Button( "Open" ) ) {
            NFD_Init( );

            nfdu8char_t* outPath;
            // nfdu8filteritem_t filters[1] = { { "GTA SAN ANDREAS SAVE FILE", "b" } };
            // nfdopendialogu8args_t args = { 0 };
            // args.filterList = filters;
            // args.filterCount = 1;
            nfdresult_t result = NFD_OpenDialogU8( &outPath, nullptr, 0, nullptr ); //_With( &outPath, &args );
            if ( result == NFD_OKAY ) {
                std::string path( outPath );
                NFD_FreePathU8( outPath );
                m_file.open( path );
                if ( m_file.is_open( ) ) {
                    m_file_path = path;
                    m_file_size = utils::format_file_size( fs::file_size( m_file_path ) );
                    m_file_name = fs::path( path ).filename( ).string( );
                    m_is_file_open = true;
                    Notify::show_notification( "File Picker", "File loaded successfully!", 3000 );
                } else {
                    Notify::show_notification( "File Picker", "File failed to load!", 3000 );
                }
            } else if ( result == NFD_CANCEL ) {
            } else {
                Notify::show_notification( "File Picker", "File failed to load!", 3000 );
            }

            NFD_Quit( );
        }
        if ( m_is_file_open ) {
            ImGui::SameLine( );
            if ( ImGui::Button( "Close" ) ) {
                m_file.close( );
                m_file_path.clear( );
                m_file_name.clear( );
                m_file_size.clear( );
                m_is_file_open = false;
            }

            ImGui::Separator( );

            ImGui::TextDisabled( "Filename: %s", m_file_name.c_str( ) );
            ImGui::TextDisabled( "Path: %s", m_file_path.c_str( ) );
            ImGui::TextDisabled(
                "Size: %s", m_file_size.c_str( ) ); // path can never be a directory, guaranteed by the NFD dependency
        }
    }

    ImGui::SameLine( );

    {
        ChildGuard dragndrop( "dragndrop", { 0.0f, 350.0f }, ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY );
        ImGui::Text( "Drag & Drop" );

        for ( const auto& p : m_dropped_paths ) {
            ImGui::TextDisabled( "Dropped: %s", p.c_str( ) );
        }
    }
}

void CDebugView::set_dropped_paths( const std::vector<std::string>& files ) {
    m_dropped_paths = files;
}

void CDebugView::on_exit( ) {
    if ( m_file.is_open( ) ) {
        m_file.close( );
        m_file_path.clear( );
    }
}

CDebugView::~CDebugView( ) { m_queue.shutdown( ); }
