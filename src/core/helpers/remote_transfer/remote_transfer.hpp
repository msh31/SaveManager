#pragma once
//Heavily linux based right now.

#include "core/helpers/zip_archive/zip_archive.hpp"
#include "core/logger/logger.hpp"
#include <libssh2.h>
#include <libssh2_sftp.h>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>
#include <future>

class RemoteTransfer {
public:
    RemoteTransfer(const std::string& dest_addr, const fs::path& backup_path, const Config& config);

    ~RemoteTransfer() {
        if(sftp_handle) {
            libssh2_sftp_close(sftp_handle);
        }
        if(sftp_session) {
            libssh2_sftp_shutdown(sftp_session);
        }
        if(session) {
             libssh2_session_disconnect(session, "Normal Shutdown");
             libssh2_session_free(session);
        }
        if(sock != LIBSSH2_INVALID_SOCKET) {
            shutdown(sock, 2);
            LIBSSH2_SOCKET_CLOSE(sock);
        }
    }

    void transfer_file(const fs::path& backup_path, const Config& config);

    // disable copying (prevent accidental double-cleanup)
    RemoteTransfer(const RemoteTransfer&) = delete;
    RemoteTransfer& operator=(const RemoteTransfer&) = delete;

    std::atomic<size_t> bytes_transferred;
    std::atomic<size_t> total_bytes;
private:
    uint32_t hostaddr;
    libssh2_socket_t sock = LIBSSH2_INVALID_SOCKET;
    struct sockaddr_in sin;
    const char *fingerprint;
    int auth_pw = 1;
    LIBSSH2_SESSION *session = NULL;
    LIBSSH2_SFTP_HANDLE *sftp_handle = nullptr;
    LIBSSH2_SFTP *sftp_session = nullptr;
};
