#include "watcher.hpp"

#if defined(__linux__)
#include <sys/inotify.h>
#include <sys/signalfd.h>
#include <signal.h>
#include <poll.h>

#define EVENT_SIZE  (sizeof(struct inotify_event))
#define BUFFER_LEN  (1024 * (EVENT_SIZE + 16)) 
#endif

Watcher::Watcher(std::function<void(const fs::path&, uint32_t)> fun) {
    m_fun = fun;
#if defined(__linux__)
    m_notify_fd = inotify_init();

    if (m_notify_fd == -1) {
        SPDLOG_ERROR("inotify_init failed");
        return;
    }

    sigset_t mask;
    sigemptyset (&mask);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGINT);
    if(sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
        SPDLOG_ERROR("sigprocmask: {}", EXIT_FAILURE);
        return;
    }

    m_signal_fd = signalfd(-1, &mask, 0);
    if(m_signal_fd == -1) {
        SPDLOG_ERROR("signalfd: {}", EXIT_FAILURE);
        return;
    }
#endif
}

void Watcher::shutdown() {
#if defined(__linux__)
    if(m_notify_fd != -1) {
        for (auto& descriptor : m_watch_descriptors) {
            inotify_rm_watch(m_notify_fd, descriptor.second);
        }

        close(m_notify_fd); 
        m_notify_fd = -1;
    }
    if(m_signal_fd != -1) {
        close(m_signal_fd); 
        m_signal_fd = -1;
    }
#endif
}

bool Watcher::add_watch(const fs::path& path) {
#if defined(__linux__)
    for (auto& entry : fs::recursive_directory_iterator(path, fs::directory_options::skip_permission_denied)) {
        if (entry.is_directory()) add_watch(entry.path());
    }
    int wd = inotify_add_watch(m_notify_fd, path.c_str(), IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO | IN_CLOSE_WRITE);
    if (wd == -1) {
        SPDLOG_ERROR("inotify_add_watch failed: {}", EXIT_FAILURE);
        return false;
    }
    m_watch_descriptors[path] = wd;
    m_wd_to_path.insert({wd, path});
    return true;
#endif
    return false; //tmp
}

bool Watcher::remove_watch(const fs::path& path) {
#if defined(__linux__)
    if (auto it = m_watch_descriptors.find(path); it != m_watch_descriptors.end()) {
        inotify_rm_watch(m_notify_fd, it->second);
        m_wd_to_path.erase(it->second);
        m_watch_descriptors.erase(it);
        return true;
    }
#endif
    return false;
}

void Watcher::run() {
#if defined(__linux__)
    struct pollfd fds[2];
    fds[0].fd = m_notify_fd; 
    fds[0].events = POLLIN;
    fds[1].fd = m_signal_fd;
    fds[1].events = POLLIN;

    struct signalfd_siginfo fdsi;
    char buffer[BUFFER_LEN];
#endif

    while (true) {
#if defined(__linux__)
        int ret = poll(fds, 2, -1);
        if (ret == -1) {
            SPDLOG_ERROR("poll failed");
            break;
        }

        if (fds[1].revents & POLLIN) {
            read(m_signal_fd, &fdsi, sizeof(fdsi));
            SPDLOG_WARN("Received signal {}, shutting down", fdsi.ssi_signo);
            break;
        }

        if (fds[0].revents & POLLIN) {
            ssize_t bytes_read = read(m_notify_fd, buffer, BUFFER_LEN);
            if (bytes_read == -1) {
                SPDLOG_ERROR("read failed");
                break;
            }

            for (char *ptr = buffer; ptr < buffer + bytes_read; ) {
                struct inotify_event *event = reinterpret_cast<inotify_event*>(ptr);

                if (!(event->mask & IN_ISDIR)) {
                    if (auto it = m_wd_to_path.find(event->wd); it != m_wd_to_path.end()) {
                        m_fun(it->second / event->name, event->mask);
                    }
                }
                ptr += EVENT_SIZE + event->len;
            }
        }
#endif
    }
}
