#include "dashboard_view.hpp"
#include <backend/utils.hpp>

#include <frontend/components/spinner.hpp>
#include <frontend/notification/notification.hpp>

void CDashboardView::on_enter( ) {
    std::vector<CacheData> data{
        { "headphones", 69.5f, { 100, 89, 75 } }, { "gaming mouse", 49.99f, { 120, 95, 80 } } };

    m_cache.refresh( [data] {
        std::this_thread::sleep_for( std::chrono::seconds( 2 ) );
        return data;
    } );
};

void CDashboardView::render( ) {
    auto cache = m_cache.get( );

    if ( m_cache.is_refreshing( ) ) {
        Spinner::render( );
    } else {
        for ( const auto& entry : cache ) {
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

void CDashboardView::on_exit( ) {}
CDashboardView::~CDashboardView( ) {}
