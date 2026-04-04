#pragma once

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

    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void success(const std::string& message);
    void debug(const std::string& message);
    void fatal(const std::string& message);

private:
    void log(const std::string& level, const std::string& message);

    std::ofstream logFile;

    std::mutex log_mutex;
};

inline logger& get_logger() {
    return logger::get_logger();
}
