#pragma once
#include <backend/cache.hpp>
#include <frontend/views/base_view.hpp>

class CDashboardView : public CBaseView {
    public:
        ~CDashboardView( ) override;
        void render( ) override;
        void on_enter( ) override;
        void on_exit( ) override;

    private:
        // tmp
        struct CacheData {
                std::string      name;
                float            floaty;
                std::vector<int> numbers;
        };

        CCache<std::vector<CacheData>> m_cache;
};
