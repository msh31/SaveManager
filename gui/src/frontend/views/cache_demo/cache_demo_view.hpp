#pragma once
#include <backend/cache.hpp>
#include <frontend/views/base_view.hpp>

class CCacheDemoView : public CBaseView {
    public:
        ~CCacheDemoView( ) override = default;
        void render( ) override;
        void on_enter( ) override;
        void on_exit( ) override;

    private:
        struct CacheData {
                std::string      name;
                float            floaty;
                std::vector<int> numbers;
        };

        CCache<std::vector<CacheData>> m_cache;
};
