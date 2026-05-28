#include "server.hpp"

void CServer::run( ) {
    fs::remove( paths::socket( ) );
    m_server_address.sun_family = AF_UNIX;
    std::strncpy( m_server_address.sun_path, paths::socket( ).string( ).c_str( ), sizeof( m_server_address.sun_path ) );

    if ( bind( m_socket, SOCKADDR_CAST & m_server_address, sizeof( m_server_address ) ) == SOCKET_ERROR ) {
        throw std::runtime_error( "failed to bind to socket!" );
    }

    if ( listen( m_socket, 5 ) == SOCKET_ERROR ) {
        throw std::runtime_error( "failed to listen to socket!" );
    }

    SPDLOG_INFO( "server is listening for connections!" );

    while ( true ) {
        sockaddr_un client_address;
        SOCKLEN_T   client_address_size = sizeof( client_address );

        SOCKET client_socket = accept( m_socket, SOCKADDR_CAST & client_address, &client_address_size );

        if ( client_socket == INVALID_SOCKET ) {
            SPDLOG_INFO( "Failed to accept client: {}", client_socket );
            continue;
        }

        if ( client_socket != INVALID_SOCKET ) {
            std::thread client_thread( &CServer::handle_client, this, client_socket );
            client_thread.detach( ); // TODO: handle gracefully
        }
    }
}

// private
void CServer::handle_client( SOCKET client_socket ) {
    // TODO
}
