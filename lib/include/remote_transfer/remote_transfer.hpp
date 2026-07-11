#pragma once
#include <libssh2.h>
#include <libssh2_sftp.h>

#include <types.hpp>

namespace fs = std::filesystem;

class CConfig;

// copied from
// https://git.marco007.dev/marco/http-server/src/commit/db41ab8f0126ed57b257face7c396c08d0999da9/socket_wrapper.hpp
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <winsock2.h>
    #include <ws2tcpip.h>

    #define CLOSE_SOCKET( s ) closesocket( s )
    #define SOCKADDR_CAST ( SOCKADDR* )
    #define SOCKET_ERROR ( -1 )
    #define SOCKLEN_T int
#else
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <unistd.h>

    #define CLOSE_SOCKET( s ) close( s )
    #define SOCKET int
    #define INVALID_SOCKET ( -1 )
    #define SOCKADDR_CAST ( struct sockaddr* )
    #define SOCKET_ERROR ( -1 )
    #define SOCKLEN_T socklen_t
#endif

class CRemoteTransfer {
    public:
        CRemoteTransfer( );
        ~CRemoteTransfer( ) { disconnect( ); }

        bool connect( const std::string& dest_addr, CConfig& config, bool auth_pw, const std::string& key_passphrase );
        bool disconnect( );
        bool upload_file( const fs::path& backup_path, const std::string& remote_path, const CConfig& config );
        bool download_file( const fs::path& backup_path, const CConfig& config );
        std::vector<RemoteEntry> list_directory( const std::string& path );

        // disable copying (prevent accidental double-cleanup)
        CRemoteTransfer( const CRemoteTransfer& ) = delete;
        CRemoteTransfer& operator=( const CRemoteTransfer& ) = delete;

        std::atomic<size_t> m_bytes_transferred = 0;
        std::atomic<size_t> m_total_bytes = 0;

    private:
        uint32_t m_hostaddr;
        libssh2_socket_t m_sock = LIBSSH2_INVALID_SOCKET;
        struct sockaddr_in m_sin;
        std::string m_fingerprint;
        LIBSSH2_SESSION* m_session = nullptr;
        LIBSSH2_SFTP_HANDLE* m_sftp_handle = nullptr;
        LIBSSH2_SFTP* m_sftp_session = nullptr;

        std::string hex_encode( const unsigned char* data, size_t len ) {
            static constexpr char digits[] = "0123456789abcdef";
            std::string out;
            out.reserve( len * 2 );
            for ( size_t i = 0; i < len; i++ ) {
                out.push_back( digits[data[i] >> 4] );
                out.push_back( digits[data[i] & 0x0F] );
            }
            return out;
        }
};
