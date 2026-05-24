#pragma once
#include "backend/text_pipeline/text_pipeline.hpp"
#include <frontend/views/base_view.hpp>

class CPipelineView : public CBaseView {
    public:
        CPipelineView( );
        ~CPipelineView( ) override;

        void render( ) override;
        void on_enter( ) override;
        void on_exit( ) override;

    private:
        CTextPipeline                             m_pipeline;
        std::string                               m_input;
        std::expected<std::string, PipelineError> m_result;
        bool                                      m_has_logged_error = false;

        std::string_view pe_to_string( PipelineError e );
};
