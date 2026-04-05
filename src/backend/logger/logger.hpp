#pragma once
#include "backend/utils/paths.hpp"

enum log_level {
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

    bool consoleLoggingEnabled = false;
    bool fileLoggingEnabled = true;

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
            case INF:
                return "INF";
            case SUC:
                return "SUC";
            case WRN:
                return "WRN";
            case ERR:
                return "ERR";
            case DBG:
                return "DBG";
            case FTL:
                return "FTL";
            default:
                return "";
        }
    }

    template<typename... Args>
    void log(log_level level, std::format_string<Args...> fmt, Args&&... args) {
        std::lock_guard<std::mutex> lock(log_mutex);
        if (fileLoggingEnabled) {
            if (!logFile.is_open()) { // this is called a lazy init apparently
                logFile.open(paths::log_file(), std::ios::app);

                if (!logFile.is_open()) {
                    return;
                }
            }

            logFile << "[" << level_to_str(level) << "] " << std::format(fmt, std::forward<Args>(args)...) << "\n";
            logFile.flush();
        }
    }

    std::ofstream logFile;

    std::mutex log_mutex;
};

inline logger& get_logger() {
    return logger::get_logger();
}
