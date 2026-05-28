#include "pipeline_demo.hpp"
#include <backend/text_pipeline/text_pipeline.hpp>

CPipelineView::CPipelineView( ) {
    m_pipeline.build( );
    m_input  = "     The word 'LEPEL' is a palindrome     ";
    m_result = m_pipeline.run( m_input );
};

// this is so ugly, and you should NOT do it like this in a real app
void CPipelineView::render( ) {
    ImGui::Text( "This is a pipeline view, a demo where you run a series of operations using state" );

    if ( ImGui::Checkbox( "Truncate", &m_pipeline.m_truncate ) ) {
        m_pipeline.build( );
        m_result = m_pipeline.run( m_input );
    }
    if ( ImGui::Checkbox( "Lowercase", &m_pipeline.m_lowercase ) ) {
        m_pipeline.build( );
        m_result = m_pipeline.run( m_input );
    }
    if ( ImGui::Checkbox( "Reverse", &m_pipeline.m_reverse ) ) {
        m_pipeline.build( );
        m_result = m_pipeline.run( m_input );
    }
    if ( ImGui::Checkbox( "Strip all whitespace", &m_pipeline.m_strip_whitespace ) ) {
        m_pipeline.build( );
        m_result = m_pipeline.run( m_input );
    }

    if ( ImGui::InputText( "##input", &m_input ) ) {
        if ( !m_input.empty( ) ) m_result = m_pipeline.run( m_input );
    }

    if ( m_result ) {
        ImGui::Text( "Output: %s", m_result->c_str( ) );
        m_has_logged_error = false;
    } else {
        switch ( m_result.error( ) ) {
        case EMPTY_INPUT:
            break;
        case PROCESSING_FAILURE:
            if ( !m_has_logged_error ) {
                SPDLOG_ERROR( "pipeline error: {}", pe_to_string( m_result.error( ) ) );
                m_has_logged_error = true;
            }
            break;
        case INVALID_STATE:
            throw std::runtime_error( "Pipeline is in an invalid state!" );
        }
        return;
    }
}

std::string_view CPipelineView::pe_to_string( PipelineError e ) {
    switch ( e ) {
    case PipelineError::EMPTY_INPUT:
        return "Pipeline was given no input, this is not a big problem.";
    case PipelineError::PROCESSING_FAILURE:
        return "Pipeline failed during processign the input, this is a problem.";
    case PipelineError::INVALID_STATE:
        return "Pipeline has an invalid state, the program cannot continue.";
    }
}

void CPipelineView::on_enter( ) {}
void CPipelineView::on_exit( ) {}
CPipelineView::~CPipelineView( ) {};
