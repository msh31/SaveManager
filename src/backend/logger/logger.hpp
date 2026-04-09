#pragma once
#include "backend/utils/paths.hpp"

enum class log_level {
    INF = 1,
    SUC,
    WRN,
    ERR,
    FTL,
    DBG,
};


class logger
{
public:
    logger();
    ~logger();

    static logger& get_logger() {
        static logger instance;
        return instance;
    }

    void trim();

    template<typename... Args>
    void info(std::format_string<Args...> fmt, Args&&... args) {
        log(log_level::INF, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void warning(std::format_string<Args...> fmt, Args&&... args) {
        log(log_level::WRN, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void error(std::format_string<Args...> fmt, Args&&... args) {
        log(log_level::ERR, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void success(std::format_string<Args...> fmt, Args&&... args) {
        log(log_level::SUC, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void debug(std::format_string<Args...> fmt, Args&&... args) {
        log(log_level::DBG, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void fatal(std::format_string<Args...> fmt, Args&&... args) {
        log(log_level::FTL, fmt, std::forward<Args>(args)...);
    }

private:
    std::string_view level_to_str(log_level level) {
        switch (level) {
            case log_level::INF:
                return "INF";
            case log_level::SUC:
                return "SUC";
            case log_level::WRN:
                return "WRN";
            case log_level::ERR:
                return "ERR";
            case log_level::DBG:
                return "DBG";
            case log_level::FTL:
                return "FTL";
            default:
                return "";
        }
    }

    template<typename... Args>
    void log(log_level level, std::format_string<Args...> fmt, Args&&... args) {
        std::lock_guard<std::mutex> lock(log_mutex);
        if (!log_file.is_open()) { //lazy init 
            log_file.open(paths::log_file(), std::ios::app);

            if (!log_file.is_open()) {
                return;
            }
        }

        log_file << "[" << level_to_str(level) << "] " << std::format(fmt, std::forward<Args>(args)...) << "\n";
        log_file.flush();
    }

    std::ofstream log_file;
    std::mutex log_mutex;
};

inline logger& get_logger() {
    return logger::get_logger();
}
