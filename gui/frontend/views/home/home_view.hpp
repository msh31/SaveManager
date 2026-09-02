#pragma once
#include <frontend/views/base_view.hpp>

class CHomeView : public CBaseView {
    public:
        ~CHomeView( ) override;
        void render( ) override;
        void on_enter( ) override;
        void on_exit( ) override;

    private:
        
};
