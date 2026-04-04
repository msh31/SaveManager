#include "logger.hpp"
#include "core/helpers/paths.hpp"

//TODO: REFACTOR

logger::logger() {
	if (fileLoggingEnabled) {
		logFile.open(paths::log_file(), std::ios::app);
        trim();
	}
}

logger::~logger() {
	if (logFile.is_open()) {
		logFile.close();
	}
}

void logger::trim() {
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
			logFile.open(paths::log_file(), std::ios::app);

			if (!logFile.is_open()) {
				return;
			}
		}

		logFile << "[" << level << "] " << message << "\n";
		logFile.flush();
	}
}
