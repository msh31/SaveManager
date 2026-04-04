#pragma once
#include <libssh2.h>
#include <libssh2_sftp.h>

namespace fs = std::filesystem;

class Config;

//copied from https://git.marco007.dev/marco/http-server/src/commit/db41ab8f0126ed57b257face7c396c08d0999da9/socket_wrapper.hpp
#ifdef _WIN32
    #include <winsock2.h> 
    #include <ws2tcpip.h>

    #define CLOSE_SOCKET(s) closesocket(s)
    #define SOCKADDR_CAST (SOCKADDR *)
    #define SOCKET_ERROR (-1)
    #define SOCKLEN_T int 
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>

    #define CLOSE_SOCKET(s) close(s)
    #define SOCKET int
    #define INVALID_SOCKET (-1)
    #define SOCKADDR_CAST (struct sockaddr *)
    #define SOCKET_ERROR (-1)
    #define SOCKLEN_T socklen_t
#endif

class RemoteTransfer {
public:
    RemoteTransfer();
    ~RemoteTransfer() {
        disconnect();
    }

    bool connect(const std::string& dest_addr, const Config& config, bool auth_pw, const std::string& key_passphrase);
    bool disconnect();
    void upload_file(const fs::path& backup_path, const std::string& remote_path, const Config& config);
    void download_file(const fs::path& backup_path, const Config& config);
    std::vector<RemoteEntry> list_directory(const std::string& path);

    // disable copying (prevent accidental double-cleanup)
    RemoteTransfer(const RemoteTransfer&) = delete;
    RemoteTransfer& operator=(const RemoteTransfer&) = delete;

    std::atomic<size_t> bytes_transferred = 0;
    std::atomic<size_t> total_bytes = 0;
private:
    uint32_t hostaddr;
    libssh2_socket_t sock = LIBSSH2_INVALID_SOCKET;
    struct sockaddr_in sin;
    const char *fingerprint;
    LIBSSH2_SESSION *session = nullptr;
    LIBSSH2_SFTP_HANDLE *sftp_handle = nullptr;
    LIBSSH2_SFTP *sftp_session = nullptr;
};
