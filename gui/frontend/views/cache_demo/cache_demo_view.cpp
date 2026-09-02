#include "cache_demo_view.hpp"
#include <frontend/childguard.hpp>

void CCacheDemoView::on_enter( ) {
    m_cache.refresh( [seed = m_seed_data]( TaskControl& control ) mutable {
        for ( int i = 0; i < 5; i++ ) {
            std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
            control.progress.store( ( i + 1 ) / 5.0f );
        }
        return std::move( seed );
    } );
};

void CCacheDemoView::render( ) {
    const auto& mem_cache = m_cache.get( );
    const auto& dsk_cache = m_disk_cache.get( );

    {
        ChildGuard memcache( "memcache", { 0.0f, 0.0f } );
        ImGui::Text( "Memory Cache Demo | Using a fake timer to simulate the work" );
        if ( m_cache.is_refreshing( ) ) {
            ImGui::ProgressBar( m_cache.progress( ) );
        } else {
            for ( const auto& entry : mem_cache ) {
                ImGui::TextColored( ImColor( 49, 206, 234 ).Value, "Item Name: %s", entry.name.c_str( ) );
                ImGui::SameLine( );
                ImGui::TextColored( ImColor( 50, 185, 18 ).Value, "Price: %f", entry.floaty );
                ImGui::SameLine( );
                ImGui::Text( " History: " );
                ImGui::SameLine( );
                for ( const auto& n : entry.numbers ) {
                    ImGui::TextColored( ImColor( 100, 100, 100 ).Value, "$%d,", n );
                    ImGui::SameLine( );
                }
                ImGui::NewLine( );
            }
        }
    }

    ImGui::SameLine( );

    {
        ChildGuard dskcache( "dskcache", { 0.0f, 0.0f } );
        ImGui::Text( "Disk Cache Demo | Using a fake timer to simulate the work" );

        if ( m_disk_cache.is_refreshing( ) ) {
            ImGui::ProgressBar( m_disk_cache.progress( ) );

            if ( ImGui::Button( "Cancel" ) ) {
                m_disk_cache.request_cancel( );
            }
        } else {
            if ( ImGui::Button( "Refresh" ) ) {
                m_disk_cache.refresh( [seed = m_seed_data]( TaskControl& control ) mutable {
                    for ( int i = 0; i < 5; i++ ) {
                        if ( control.cancel_requested.load( ) ) throw TaskCancelled{ };
                        std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
                        control.progress.store( ( i + 1 ) / 5.0f );
                    }
                    return std::move( seed );
                } );
            }

            for ( const auto& entry : dsk_cache ) {
                ImGui::TextColored( ImColor( 49, 206, 234 ).Value, "Item Name: %s", entry.name.c_str( ) );
                ImGui::SameLine( );
                ImGui::TextColored( ImColor( 50, 185, 18 ).Value, "Price: %f", entry.floaty );
                ImGui::SameLine( );
                ImGui::Text( " History: " );
                ImGui::SameLine( );
                for ( const auto& n : entry.numbers ) {
                    ImGui::TextColored( ImColor( 100, 100, 100 ).Value, "$%d,", n );
                    ImGui::SameLine( );
                }
                ImGui::NewLine( );
            }
        }
    }
}

void CCacheDemoView::on_exit( ) {}
