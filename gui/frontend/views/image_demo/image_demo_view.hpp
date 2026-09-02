#pragma once
#include <backend/image_manager/image_manager.hpp>
#include <frontend/views/base_view.hpp>

class CImageDemoView : public CBaseView {
    public:
        ~CImageDemoView( ) override = default;
        void render( ) override;
        void on_enter( ) override;
        void on_exit( ) override;

    private:
        ImageData m_embedded;
};
