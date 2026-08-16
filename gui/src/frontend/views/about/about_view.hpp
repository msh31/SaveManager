#pragma once
#include <frontend/views/base_view.hpp>

class CAboutView : public CBaseView {
    public:
        ~CAboutView( ) override = default;
        void render( ) override;
        void on_enter( ) override;
        void on_exit( ) override;

    private:
        bool m_logo_loaded = false;
        GLuint m_logo_tex = 0;
};
