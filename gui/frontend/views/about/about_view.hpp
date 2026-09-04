#pragma once
#include <frontend/views/base_view.hpp>
#include <backend/image_manager/image_manager.hpp>

class CAboutView : public CBaseView {
    public:
        ~CAboutView( ) override = default;
        void render( ) override;
        void on_enter( ) override;
        void on_exit( ) override;

    private:
        ImageData m_embedded;
};
