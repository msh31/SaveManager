#include "features/remote_transfer/remote_transfer.hpp"
#include "config/config.hpp"

// https://libssh2.org/examples/sftp_write.html
CRemoteTransfer::CRemoteTransfer( ) {}

bool CRemoteTransfer::connect(
    const std::string& dest_addr, const CConfig& config, bool auth_pw, const std::string& key_passphrase ) {
#ifdef _WIN32
    WSADATA wsadata;
    WSAStartup( MAKEWORD( 2, 2 ), &wsadata );
#endif

    auto fail = [this]( ) -> bool {
        disconnect( );
        return false;
    };

    int result = libssh2_init( 0 );
    if ( result ) {
        SPDLOG_ERROR( "libssh2 initialization failed: {}", result );
        return false;
    }

    m_sock = socket( AF_INET, SOCK_STREAM, 0 );
    if ( m_sock == LIBSSH2_INVALID_SOCKET ) {
        SPDLOG_ERROR( "failed to create socket!" );
        return false;
    }

    m_sin.sin_family = AF_INET;
    m_sin.sin_port = htons( 22 );
    int rc = inet_pton( AF_INET, dest_addr.c_str( ), &m_sin.sin_addr );
    if ( rc <= 0 ) {
        SPDLOG_ERROR( "Invalid address: {}", dest_addr );
        return fail( );
    }

    if ( ::connect( m_sock, SOCKADDR_CAST( &m_sin ), sizeof( struct sockaddr_in ) ) ) {
        SPDLOG_ERROR( "failed to connect to socket: {}", strerror( errno ) );
        return fail( );
    }

    m_session = libssh2_session_init( );
    if ( !m_session ) {
        SPDLOG_ERROR( "Could not initialize SSH session." );
        return fail( );
    }

    libssh2_session_set_blocking( m_session, 1 );
    result = libssh2_session_handshake( m_session, m_sock );
    if ( result ) {
        SPDLOG_ERROR( "Failure establishing SSH session: {}", result );
        return fail( );
    }

    m_fingerprint = libssh2_hostkey_hash( m_session, LIBSSH2_HOSTKEY_HASH_SHA1 );

    if ( auth_pw ) {
        if ( libssh2_userauth_password( m_session, config.sftp.username.c_str( ), config.sftp.password.c_str( ) ) ) {
            SPDLOG_ERROR( "Authentication by password failed." );
            return fail( );
        }
    } else {
        if ( libssh2_userauth_publickey_fromfile(
                 m_session, config.sftp.username.c_str( ), config.sftp.pubkey.string( ).c_str( ),
                 config.sftp.privkey.string( ).c_str( ),
                 key_passphrase.empty( ) ? nullptr : key_passphrase.c_str( ) ) ) {
            SPDLOG_ERROR( "Authentication by public key failed." );
            return fail( );
        }
    }

    m_sftp_session = libssh2_sftp_init( m_session );
    if ( !m_sftp_session ) {
        SPDLOG_ERROR( "Unable to init SFTP session" );
        return fail( );
    }

    return true;
}

bool CRemoteTransfer::disconnect( ) {
    if ( m_sftp_handle ) {
        libssh2_sftp_close( m_sftp_handle );
        m_sftp_handle = nullptr;
    }
    if ( m_sftp_session ) {
        libssh2_sftp_shutdown( m_sftp_session );
        m_sftp_session = nullptr;
    }
    if ( m_session ) {
        libssh2_session_disconnect( m_session, "Normal Shutdown" );
        libssh2_session_free( m_session );
        m_session = nullptr;
    }

    m_session = nullptr;
    m_sftp_handle = nullptr;
    m_sftp_session = nullptr;

    if ( m_sock != LIBSSH2_INVALID_SOCKET ) {
        shutdown( m_sock, 2 );
        LIBSSH2_SOCKET_CLOSE( m_sock );
        m_sock = LIBSSH2_INVALID_SOCKET;
#ifdef _WIN32
        WSACleanup( );
#endif

        return true;
    }
    return false;
}

void CRemoteTransfer::upload_file(
    const fs::path& backup_path, const std::string& remote_path, const CConfig& config ) {
    char mem[1024 * 100];
    size_t nread;
    ssize_t nwritten;
    char* ptr;

    std::string remote_file =
        remote_path + ( remote_path.back( ) == '/' ? "" : "/" ) + backup_path.filename( ).string( );
    m_sftp_handle = libssh2_sftp_open(
        m_sftp_session, remote_file.c_str( ), LIBSSH2_FXF_WRITE | LIBSSH2_FXF_CREAT | LIBSSH2_FXF_TRUNC,
        LIBSSH2_SFTP_S_IRUSR | LIBSSH2_SFTP_S_IWUSR | LIBSSH2_SFTP_S_IRGRP | LIBSSH2_SFTP_S_IROTH );
    if ( !m_sftp_handle ) {
        SPDLOG_ERROR( "Unable to open path with SFTP {}", libssh2_sftp_last_error( m_sftp_session ) );
        return;
    }

    std::ifstream file( backup_path, std::ios::binary );
    if ( !file.is_open( ) ) {
        libssh2_sftp_close_handle( m_sftp_handle );
        m_sftp_handle = nullptr;
        SPDLOG_ERROR( "Could not open backup path with SFTP" );
        return;
    }

    m_total_bytes = fs::file_size( backup_path );
    do {
        file.read( mem, sizeof( mem ) );
        if ( file.gcount( ) <= 0 ) {
            break;
        }
        ptr = mem;
        nread = file.gcount( );

        do {
            nwritten = libssh2_sftp_write( m_sftp_handle, ptr, nread );

            if ( nwritten < 0 ) {
                break;
            }
            ptr += nwritten;
            nread -= (size_t)nwritten;
            m_bytes_transferred += nwritten;
        } while ( nread );
    } while ( file.gcount( ) > 0 );

    SPDLOG_INFO( "File has been uploaded!" );
    libssh2_sftp_close_handle( m_sftp_handle );
    m_sftp_handle = nullptr;
}

std::vector<RemoteEntry> CRemoteTransfer::list_directory( const std::string& path ) {
    LIBSSH2_SFTP_HANDLE* handle = libssh2_sftp_opendir( m_sftp_session, path.c_str( ) );
    if ( !handle ) {
        return { };
    }

    char buffer[512];
    std::vector<RemoteEntry> entry;
    LIBSSH2_SFTP_ATTRIBUTES attrs;

    ssize_t len;
    while ( ( len = libssh2_sftp_readdir( handle, buffer, sizeof( buffer ), &attrs ) ) > 0 ) {
        entry.push_back(
            { std::string( buffer, static_cast<size_t>( len ) ), LIBSSH2_SFTP_S_ISDIR( attrs.permissions ) != 0 } );
    }

    libssh2_sftp_closedir( handle );
    return entry;
}

void CRemoteTransfer::download_file( const fs::path& backup_path, const CConfig& config ) {
    char mem[1024 * 100];

    fs::path local_path = paths::backup_dir( ) / backup_path.filename( );
    m_sftp_handle = libssh2_sftp_open( m_sftp_session, backup_path.string( ).c_str( ), LIBSSH2_FXF_READ, 0 );
    if ( !m_sftp_handle ) {
        SPDLOG_ERROR( "Unable to open path with SFTP: {}", libssh2_sftp_last_error( m_sftp_session ) );
        return;
    }

    std::ofstream file( local_path, std::ios::binary );
    if ( !file.is_open( ) ) {
        SPDLOG_ERROR( "Could not open backup path with SFTP" );
        libssh2_sftp_close( m_sftp_handle );
        m_sftp_handle = nullptr;
        return;
    }

    ssize_t rc;
    while ( ( rc = libssh2_sftp_read( m_sftp_handle, mem, sizeof( mem ) ) ) > 0 ) {
        if ( !file.write( mem, rc ) ) break;
        m_bytes_transferred += rc;
    }
    SPDLOG_INFO( "File has been downloaded!" );
    libssh2_sftp_close( m_sftp_handle );
    m_sftp_handle = nullptr;
}
