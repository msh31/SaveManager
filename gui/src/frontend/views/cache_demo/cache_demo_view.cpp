#include "cache_demo_view.hpp"
#include <frontend/components/spinner.hpp>

void CCacheDemoView::on_enter( ) {
    std::vector<CacheData> data{
        { "headphones", 69.5f, { 100, 89, 75 } },
        { "gaming mouse", 49.99f, { 120, 95, 80 } },
        { "mechanical keyboard", 129.0f, { 140, 110, 92 } },
        { "webcam", 89.5f, { 98, 85, 70 } },
        { "usb microphone", 159.99f, { 130, 100, 88 } },
        { "gaming monitor", 349.99f, { 180, 145, 120 } },
        { "graphics card", 799.0f, { 250, 210, 180 } },
        { "ssd drive", 119.99f, { 115, 90, 75 } },
        { "external hard drive", 89.0f, { 105, 84, 72 } },
        { "smartphone", 999.99f, { 240, 200, 175 } },
        { "tablet", 549.5f, { 190, 160, 135 } },
        { "smartwatch", 299.99f, { 145, 120, 98 } },
        { "wireless earbuds", 149.99f, { 135, 108, 90 } },
        { "bluetooth speaker", 79.99f, { 100, 82, 68 } },
        { "vr headset", 449.0f, { 210, 175, 150 } },
        { "gaming chair", 259.99f, { 175, 145, 118 } },
        { "laptop stand", 39.99f, { 88, 70, 58 } },
        { "router", 129.99f, { 125, 102, 84 } },
        { "stream deck", 149.0f, { 132, 109, 91 } },
        { "capture card", 189.99f, { 155, 128, 106 } },
        { "power bank", 59.99f, { 95, 78, 65 } } };

    m_cache.refresh( [data] {
        std::this_thread::sleep_for( std::chrono::seconds( 2 ) );
        return data;
    } );
};

void CCacheDemoView::render( ) {
    if ( m_cache.is_refreshing( ) ) {
        Spinner::render( );
    } else {
        auto cache = m_cache.get( );

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

void CCacheDemoView::on_exit( ) {}
