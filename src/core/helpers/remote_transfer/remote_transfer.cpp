#include "remote_transfer.hpp"

//https://libssh2.org/examples/sftp_write.html
RemoteTransfer::RemoteTransfer(const std::string& dest_addr, const fs::path& backup_path, const Config& config) {
    int result = libssh2_init(0);
    if(result) {
        get_logger().error("libssh2 initialization failed!" + std::to_string(result));
        return;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == LIBSSH2_INVALID_SOCKET) {
        get_logger().error("failed to create socket!");
        return;
    }

    sin.sin_family = AF_INET;
    sin.sin_port = htons(22);
    sin.sin_addr.s_addr = inet_pton(AF_INET, dest_addr.c_str(), &sin.sin_addr);
    if(connect(sock, (struct sockaddr*)(&sin), sizeof(struct sockaddr_in))) {
        get_logger().error("failed to connect to socket: " + std::string(strerror(errno)));
        return;
    }

    session = libssh2_session_init();
    if(!session) {
        get_logger().error("Could not initialize SSH session.");
        return;
    }

    libssh2_session_set_blocking(session, 1);
    result = libssh2_session_handshake(session, sock);
    if(result) {
        get_logger().error("Failure establishing SSH session: " + std::to_string(result));
        return;
    }

    fingerprint = libssh2_hostkey_hash(session, LIBSSH2_HOSTKEY_HASH_SHA1);

    if(auth_pw) {
        if(libssh2_userauth_password(session, config.sftp.username.c_str(), config.sftp.password.c_str())) {
            get_logger().error("Authentication by password failed.");
            return;
        }
    }
    else {
        if(libssh2_userauth_publickey_fromfile(session, config.sftp.username.c_str(), config.sftp.pubkey.string().c_str(), config.sftp.privkey.string().c_str(), config.sftp.password.c_str())) {
            get_logger().error("Authentication by public key failed.");
            return;
        }
    }

    sftp_session = libssh2_sftp_init(session);
    if(!sftp_session) {
        get_logger().error("Unable to init SFTP session");
        return;
    }

    sftp_handle = libssh2_sftp_open(sftp_session, backup_path.string().c_str(),
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

    char mem[1024 * 100];
    size_t nread;
    ssize_t nwritten;
    char *ptr;

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
        } while(nread);
    } while(file.gcount() > 0);
}
