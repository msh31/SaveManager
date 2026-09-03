#pragma once

class CModalBase {
    public:
        explicit CModalBase( std::string title ) : m_title( std::move( title ) ) {};
        virtual ~CModalBase( ) = default;

        void render( );

    protected:
        void request_open( ) { m_should_open = true; }
        virtual void render_content( ) = 0;

    private:
        std::string m_title = { };
        bool m_should_open = false;
};