#pragma once
#include <frontend/views/base_view.hpp>

class CEditorView : public CBaseView {
    public:
        ~CEditorView( ) override = default;
        void render( ) override;
        void on_enter( ) override {}
        void on_exit( ) override {}
};
