#pragma once
#include <utils/cache.hpp>
#include <utils/disk_cache.hpp>

#include <frontend/views/base_view.hpp>

class CCacheDemoView : public CBaseView {
    public:
        CCacheDemoView( ) : m_disk_cache( paths::demo_disk_cache_file( ) ) {}
        ~CCacheDemoView( ) override = default;
        void render( ) override;
        void on_enter( ) override;
        void on_exit( ) override;

    private:
        struct CacheData {
                std::string name;
                float floaty;
                std::vector<int> numbers;
                NLOHMANN_DEFINE_TYPE_INTRUSIVE( CacheData, name, floaty, numbers );
        };

        CCache<std::vector<CacheData>> m_cache;
        CDiskCache<std::vector<CacheData>> m_disk_cache;

        std::vector<CacheData> m_seed_data{
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
};