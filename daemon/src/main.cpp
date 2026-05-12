#if defined(__linux__)

#include <sys/inotify.h>
#include <sys/signalfd.h>
#include <signal.h>
#include <utils/paths.hpp>

#define EVENT_SIZE  (sizeof(struct inotify_event))
#define BUFFER_LEN  (1024 * (EVENT_SIZE + 16)) 

int main() {
    int fd = inotify_init();
    if (fd == -1) {
        SPDLOG_ERROR("inotify_init failed");
        return EXIT_FAILURE;
    }
    SPDLOG_DEBUG("Inotify instance initialized. File descriptor: {}", fd);

    sigset_t mask;
    struct signalfd_siginfo fdsi;
    sigemptyset (&mask);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGINT);
    if(sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
        SPDLOG_ERROR("sigprocmask: {}", EXIT_FAILURE);
        return EXIT_FAILURE;
    }

    int sfd = signalfd(-1, &mask, 0);
    if(sfd == -1) {
        SPDLOG_ERROR("signalfd: {}", EXIT_FAILURE);
        return EXIT_FAILURE;
    }

    std::string file_name = (paths::home_dir() / "test.savemgr").string();
    int wd = inotify_add_watch(fd, file_name.c_str(), IN_MODIFY);
    if (wd == -1) {
        SPDLOG_ERROR("inotify_add_watch failed");
        close(fd); 
        return EXIT_FAILURE;
    }
    SPDLOG_DEBUG("Watching file: {} (watch descriptor: {})", file_name.c_str(), wd);

    SPDLOG_DEBUG("Waiting for changes to '{}'..", file_name.c_str());
    char buffer[BUFFER_LEN];
    while (true) {
        ssize_t bytes_read = read(fd, buffer, BUFFER_LEN);
        if (bytes_read == -1) {
            SPDLOG_ERROR("read failed");
            return EXIT_FAILURE;
        }

        for (char *ptr = buffer; ptr < buffer + bytes_read; ) {
            struct inotify_event *event = reinterpret_cast<inotify_event*>(ptr);

            if (event->mask & IN_MODIFY) {
                SPDLOG_DEBUG("\nEvent detected on '{}'!", file_name.c_str());
                SPDLOG_DEBUG("Watch descriptor: {}", event->wd);
                SPDLOG_DEBUG("Event type: File content modified (IN_MODIFY)");
            }
            ptr += EVENT_SIZE + event->len;
        }
    }

    inotify_rm_watch(fd, wd);
    close(fd);
    return 0;
}

// #elif defined(__APPLE__)
//
#else
int main() {
    std::println("this is not linux!");
    return 0;
} 
#endif

