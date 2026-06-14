/*
 * TAKEN FROM HERE AND MODIFIED FOR THIS PROJECT
 *  https://git.marco007.dev/marco/http-server
 *
 */

#pragma once

#ifdef _WIN32
// clang-format off
    #include <winsock2.h> //enables network programming
    #include <afunix.h>   //for AF_UNIX support on windows
// clang-format on

    #define CLOSE_SOCKET( s ) closesocket( s )
    #define SOCKADDR_CAST ( SOCKADDR* )
    #define SOCKET_ERROR ( -1 )
    #define SOCKLEN_T int
#else
    #include <sys/socket.h>
    #include <sys/un.h>
    #include <unistd.h>

    #define CLOSE_SOCKET( s ) close( s )
    #define SOCKET int
    #define INVALID_SOCKET ( -1 )
    #define SOCKADDR_CAST ( struct sockaddr* )
    #define SOCKET_ERROR ( -1 )
    #define SOCKLEN_T socklen_t
#endif

class CSocket {
    public:
        CSocket( ) {
#ifdef _WIN32
            WSADATA wsaData;

            if ( WSAStartup( MAKEWORD( 2, 2 ), &wsaData ) != 0 ) {
                m_wsa_initialized = false;
                throw std::runtime_error( "Failed to create socket!" );
            }
            m_wsa_initialized = true;
#endif
            m_sock = socket( AF_UNIX, SOCK_STREAM, 0 );
            if ( m_sock == INVALID_SOCKET ) {
                throw std::runtime_error( "Failed to create socket!" );
            }
        }
        ~CSocket( ) {
            if ( m_sock != INVALID_SOCKET ) {
                CLOSE_SOCKET( m_sock );
            }
        }

        operator SOCKET( ) const { return m_sock; }
        CSocket( const CSocket& )            = delete;
        CSocket& operator=( const CSocket& ) = delete;

    private:
        SOCKET m_sock;
        bool   m_wsa_initialized = false;
};
