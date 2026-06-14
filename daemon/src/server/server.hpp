#pragma once
#include <socket.hpp>

class CServer {
    public:
        CServer( ) = default;

        void run( ); // listen

    private:
        void        handle_client( SOCKET client_socket );
        CSocket     m_socket;
        sockaddr_un m_server_address;
};
