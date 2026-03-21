#include "remote_transfer.hpp"
#include "core/config/config.hpp"
#include "core/globals.hpp"
#include "core/logger/logger.hpp"
#include <fstream>
#include <libssh2_sftp.h>

//https://libssh2.org/examples/sftp_write.html
RemoteTransfer::RemoteTransfer() {}

bool RemoteTransfer::connect(const std::string& dest_addr, const Config& config, bool auth_pw, const std::string& key_passphrase) {
#ifdef _WIN32
    WSADATA wsadata;
    WSAStartup(MAKEWORD(2, 2), &wsadata);
#endif
    
    int result = libssh2_init(0);
    if(result) {
        get_logger().error("libssh2 initialization failed!" + std::to_string(result));
        return false;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == LIBSSH2_INVALID_SOCKET) {
        get_logger().error("failed to create socket!");
        return false;
    }

    sin.sin_family = AF_INET;
    sin.sin_port = htons(22);
    int rc = inet_pton(AF_INET, dest_addr.c_str(), &sin.sin_addr);
    if(rc <= 0) {
        get_logger().error("Invalid address: " + dest_addr);
        return false;
    }

    if(::connect(sock, SOCKADDR_CAST(&sin), sizeof(struct sockaddr_in))) {
        get_logger().error("failed to connect to socket: " + std::string(strerror(errno)));
        return false;
    }

    session = libssh2_session_init();
    if(!session) {
        get_logger().error("Could not initialize SSH session.");
        return false;
    }

    libssh2_session_set_blocking(session, 1);
    result = libssh2_session_handshake(session, sock);
    if(result) {
        get_logger().error("Failure establishing SSH session: " + std::to_string(result));
        return false;
    }

    fingerprint = libssh2_hostkey_hash(session, LIBSSH2_HOSTKEY_HASH_SHA1);

    if(auth_pw) {
        if(libssh2_userauth_password(session, config.sftp.username.c_str(), config.sftp.password.c_str())) {
            get_logger().error("Authentication by password failed.");
            return false;
        }
    } else {
        if(libssh2_userauth_publickey_fromfile(session, config.sftp.username.c_str(), config.sftp.pubkey.string().c_str(), config.sftp.privkey.string().c_str(), key_passphrase.empty() ? nullptr : key_passphrase.c_str())) {
            get_logger().error("Authentication by public key failed.");
            return false;
        }
    }

    sftp_session = libssh2_sftp_init(session);
    if(!sftp_session) {
        get_logger().error("Unable to init SFTP session");
        return false;
    }

    return true;
}

bool RemoteTransfer::disconnect() {
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

    session = nullptr;
    sftp_handle = nullptr;
    sftp_session = nullptr;

    if(sock != LIBSSH2_INVALID_SOCKET) {
        shutdown(sock, 2);
        LIBSSH2_SOCKET_CLOSE(sock);
#ifdef _WIN32
        WSACleanup();
#endif

        return true;
    }
    return false;
}

void RemoteTransfer::upload_file(const fs::path& backup_path, const Config& config) {
    char mem[1024 * 100];
    size_t nread;
    ssize_t nwritten;
    char *ptr;

    fs::path remote_file = config.sftp.remote_path / backup_path.filename();
    sftp_handle = libssh2_sftp_open(sftp_session, remote_file.string().c_str(),
                                    LIBSSH2_FXF_WRITE |
                                    LIBSSH2_FXF_CREAT |
                                    LIBSSH2_FXF_TRUNC,
                                    LIBSSH2_SFTP_S_IRUSR |
                                    LIBSSH2_SFTP_S_IWUSR |
                                    LIBSSH2_SFTP_S_IRGRP |
                                    LIBSSH2_SFTP_S_IROTH);
    if(!sftp_handle) {
        std::string error("Unable to open path with SFTP" + std::to_string(libssh2_sftp_last_error(sftp_session)));
        get_logger().error(error);
        return;
    }

    std::ifstream file(backup_path, std::ios::binary);
    if(!file.is_open()) {
        get_logger().error("Could not open backup path with SFTP");
        return;
    }

    total_bytes = fs::file_size(backup_path);
    do {
        file.read(mem, sizeof(mem));
        if(file.gcount() <= 0) {
            break;
        }
        ptr = mem;
        nread = file.gcount();

        do {
            nwritten = libssh2_sftp_write(sftp_handle, ptr, nread);

            if(nwritten < 0) {
                break;
            }
            ptr += nwritten;
            nread -= (size_t)nwritten;
            bytes_transferred += nwritten;
        } while(nread);
    } while(file.gcount() > 0);

    get_logger().success("File has been uploaded!");
}

std::vector<RemoteEntry> RemoteTransfer::list_directory(const std::string& path) {
    LIBSSH2_SFTP_HANDLE* handle = libssh2_sftp_opendir(sftp_session, path.c_str());
    if(!handle) {
        return {};
    }

    char buffer[512];
    std::vector<RemoteEntry> entry;
    LIBSSH2_SFTP_ATTRIBUTES attrs;

    while(libssh2_sftp_readdir(handle, buffer, sizeof(buffer), &attrs) > 0) {
        entry.push_back({std::string(buffer), LIBSSH2_SFTP_S_ISDIR(attrs.permissions) != 0});
    }

    libssh2_sftp_closedir(handle);
    return entry;
}

void RemoteTransfer::download_file(const fs::path& backup_path, const Config& config) {
    char mem[1024 * 100];

    fs::path remote_file = config.sftp.remote_path / backup_path.filename();
    fs::path local_path = config.settings.backup_path / backup_path.filename();
    sftp_handle = libssh2_sftp_open(sftp_session, remote_file.string().c_str(), LIBSSH2_FXF_READ, 0);
    if(!sftp_handle) {
        std::string error("Unable to open path with SFTP" + std::to_string(libssh2_sftp_last_error(sftp_session)));
        get_logger().error(error);
        return;
    }

    std::ofstream file(local_path, std::ios::binary);
    if(!file.is_open()) {
        get_logger().error("Could not open backup path with SFTP");
        return;
    }

    ssize_t rc;
    while ((rc = libssh2_sftp_read(sftp_handle, mem, sizeof(mem))) > 0) {
        file.write(mem, rc);
        bytes_transferred += rc;
    }
    get_logger().success("File has been download!");
}
