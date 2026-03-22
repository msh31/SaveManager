#include "logger.hpp"

logger::logger() {
	if (fileLoggingEnabled) {
		logFile.open(logFilePath, std::ios::app);
	}
}

logger::~logger() {
	if (logFile.is_open()) {
		logFile.close();
	}
}

void logger::info(const std::string& message) {
	log("INF", message);
}

void logger::warning(const std::string& message) {
	log("WRN", message);
}

void logger::error(const std::string& message) {
	log("ERR", message);
}

void logger::success(const std::string& message) {
	log("SUC", message);
}

void logger::debug(const std::string& message) {
	log("DBG", message);
}

void logger::fatal(const std::string& message) {
	log("FATAL", message);
}

void logger::log(const std::string& level, const std::string& message) {
    std::lock_guard<std::mutex> lock(log_mutex);
	if (fileLoggingEnabled) {
		if (!logFile.is_open()) { // this is called a lazy init apparently
			logFile.open(logFilePath, std::ios::app);

			if (!logFile.is_open()) {
				return;
			}
		}

		logFile << "[" << level << "] " << message << "\n";
		logFile.flush();
	}
}
