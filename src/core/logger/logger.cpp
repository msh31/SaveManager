#include "logger.hpp"
#include <unordered_map>

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

std::string logger::getColorForLevel(const std::string& level) {
	static const std::unordered_map<std::string, std::string> levelColors = {
		{"INF", SENTINEL_INFO},
		{"WRN", SENTINEL_WARNING},
		{"ERR", SENTINEL_ERROR},
		{"DBG", SENTINEL_SUCCESS},
		{"SUC", SENTINEL_SUCCESS},
		{"FATAL", SENTINEL_FATAL}
	};

	auto colorEntry = levelColors.find(level);
	if (colorEntry != levelColors.end()) {
		return colorEntry->second; //value from the key-value pair :D
	}

	return SENTINEL_RESET;
}

void logger::log(const std::string& level, const std::string& message) {
	std::string colorCode = getColorForLevel(level);

	if (fileLoggingEnabled) {
		if (!logFile.is_open()) { // this is called a lazy init apparently
			logFile.open(logFilePath, std::ios::app);

			if (!logFile.is_open()) {
				//std::cerr << "Error: Could not open log file at " << logFilePath << ". Disabling file logging.\n";
				fileLoggingEnabled = false;
				return;
			}
		}

		logFile << "[" << level << "] " << message << "\n";
		logFile.flush();
	}
}