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
    logger() {
        trim();
    }
    //~logger();

    static constexpr int entry_cap = 200;

    static logger& get_logger() {
        static logger instance;
        return instance;
    }

    void trim() {
        std::lock_guard<std::mutex> lock(log_mutex);
        std::ifstream in((paths::log_file()));
        std::vector<std::string> lines;
        std::string line;
        while (std::getline(in, line)) {
            lines.push_back(line);
        }
        in.close();

        if (lines.size() > 100) {
            lines = std::vector<std::string>(lines.end() - 100, lines.end());
            std::ofstream out((paths::log_file()));
            for (const auto& l : lines) {
                out << l << '\n';
            }
        }
    }

    void clear() {
        std::lock_guard<std::mutex> lock(log_mutex);
        log_entries.clear();
        std::ofstream(paths::log_file(), std::ios::trunc);
    }

    std::deque<std::string> get_entries() const {
        std::lock_guard<std::mutex> lock(log_mutex);
        return log_entries;
    }

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
    std::deque<std::string> log_entries;
    std::ofstream log_file;
    mutable std::mutex log_mutex;

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

        auto line = std::format("[{}] {}", level_to_str(level), std::format(fmt, std::forward<Args>(args)...));
        log_file << line << "\n";
        log_file.flush();
        log_entries.push_back(line);
        if (log_entries.size() > entry_cap) {
            log_entries.pop_front();
        }
    }
};

inline logger& get_logger() {
    return logger::get_logger();
}
