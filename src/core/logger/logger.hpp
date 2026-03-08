#pragma once
#include "core/helpers/paths.hpp"

#include <string>
#include <fstream>

#define SENTINEL_RESET     "\033[0m"
#define SENTINEL_ERROR     "\033[31m"
#define SENTINEL_WARNING   "\033[33m"
#define SENTINEL_INFO      "\033[36m"
#define SENTINEL_SUCCESS   "\033[32m" // (or DEBUG)
#define SENTINEL_FATAL     "\033[35m"

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

    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void success(const std::string& message);
    void debug(const std::string& message);
    void fatal(const std::string& message);

private:
    void log(const std::string& level, const std::string& message);

    std::ofstream logFile;

    fs::path logFilePath = config_dir / "savemanager.log";
    static std::string getColorForLevel(const std::string& level);
};

inline logger& get_logger() {
    return logger::get_logger();
}
