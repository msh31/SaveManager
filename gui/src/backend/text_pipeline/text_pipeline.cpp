#include "text_pipeline.hpp"

void CTextPipeline::build( ) {
    m_stages.clear( );

    if ( m_lowercase ) {
        m_stages.push_back( []( std::string s ) -> std::expected<std::string, PipelineError> {
            std::transform( s.begin( ), s.end( ), s.begin( ), ::tolower );
            return s;
        } );
    }

    if ( m_strip_whitespace ) {
        m_stages.push_back( []( std::string s ) -> std::expected<std::string, PipelineError> {
            return regex_replace( s, std::regex( "(^[ ]+)|([ \n]+$)" ), "" );
        } );
    }

    if ( m_reverse ) {
        m_stages.push_back( []( std::string s ) -> std::expected<std::string, PipelineError> {
            std::reverse( s.begin( ), s.end( ) );
            return s;
        } );
    }

    if ( m_truncate ) {
        m_stages.push_back( [this]( std::string s ) -> std::expected<std::string, PipelineError> {
            s.resize( m_max_str_len );
            return s;
        } );
    }

    m_is_built = true;
}

std::expected<std::string, PipelineError> CTextPipeline::run( std::string_view input ) {
    if ( !m_is_built ) return std::unexpected( INVALID_STATE );
    if ( m_stages.empty( ) ) return std::unexpected( EMPTY_INPUT );

    std::string current = std::string( input );
    for ( auto& stage : m_stages ) {
        auto result = stage( current );
        if ( !result ) return std::unexpected( result.error( ) );
        current = *result;
    }
    return current;
}
