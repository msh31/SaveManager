#pragma once
#include <frontend/views/base_view.hpp>

class CTransferView : public CBaseView {
    public:
        ~CTransferView( ) override = default;
        void render( ) override;
        void on_enter( ) override {}
        void on_exit( ) override {}
};
