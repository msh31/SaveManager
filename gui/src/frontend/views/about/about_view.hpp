#pragma once
#include <frontend/views/base_view.hpp>

class CAboutView : public CBaseView {
    public:
        ~CAboutView( ) override = default;
        void render( ) override;
        void on_enter( ) override {}
        void on_exit( ) override {}
};
