#pragma once
#include "save_editor/save_editor.hpp"
#include <frontend/views/base_view.hpp>

class CEditorView : public CBaseView {
    public:
        ~CEditorView( ) override = default;
        void render( ) override;
        void on_enter( ) override {}
        void on_exit( ) override {}

    private:
        SanAndreas m_san_andreas;
        std::string file_path;
};
