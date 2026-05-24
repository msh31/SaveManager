#pragma once
#include "types.hpp"

using Stage = std::function<std::expected<std::string, PipelineError>( std::string )>;

class CTextPipeline {
    public:
        void                                      build( );
        std::expected<std::string, PipelineError> run( std::string_view input );

        bool m_strip_whitespace = false;
        bool m_lowercase        = false;
        bool m_reverse          = true;
        bool m_truncate         = false;

        bool m_is_built = false;

    private:
        std::vector<Stage> m_stages;

        int m_max_str_len = 100; // whatever
};
