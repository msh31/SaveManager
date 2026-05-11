#pragma once
#include <utils/paths.hpp>
#include <constants.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/daily_file_sink.h>

template<typename Mutex>
class ringbuffer_sink : public spdlog::sinks::base_sink<Mutex> {
public:
    void clear() {
        std::lock_guard<Mutex> lock(spdlog::sinks::base_sink<Mutex>::mutex_);
        messages.clear();
    }

    std::deque<std::string> get_messages() {
        std::lock_guard<Mutex> lock(spdlog::sinks::base_sink<Mutex>::mutex_);
        return messages;
    }
private:
    std::deque<std::string> messages;
protected:
    void sink_it_(const spdlog::details::log_msg& msg) override {
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);

        if(messages.size() >= 500) messages.pop_front();
        messages.emplace_back(formatted.data(), formatted.size());
    }
    void flush_() override {}
};

using ringbuffer_sink_mt = ringbuffer_sink<std::mutex>;

inline std::shared_ptr<ringbuffer_sink_mt> g_ringbuffer_sink;

inline void init_logger() {
    auto config_dir = paths::config_dir();
    if (!fs::exists(config_dir)) {
        std::error_code ec;
        fs::create_directories(config_dir, ec);
    }

    g_ringbuffer_sink = std::make_shared<ringbuffer_sink_mt>();

    std::vector<spdlog::sink_ptr> sinks{g_ringbuffer_sink};
    if (fs::exists(config_dir)) {
        sinks.push_back(std::make_shared<spdlog::sinks::daily_file_sink_mt>(
            (paths::log_dir() / "savemanager.log").string(), 0, 0));
    }

    auto app_logger = std::make_shared<spdlog::logger>(APP_NAME, sinks.begin(), sinks.end());
    spdlog::set_default_logger(app_logger);
    spdlog::set_pattern("[%l] %d-%m-%Y %H:%M:%S - %v");
}

inline ringbuffer_sink_mt* get_ringbuffer_sink() {
    return g_ringbuffer_sink.get();
}
