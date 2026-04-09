#include "logger.hpp"

//TODO: REFACTOR

logger::logger() {
    log_file.open(paths::log_file(), std::ios::app);
    trim();
}

logger::~logger() {
	if (log_file.is_open()) {
		log_file.close();
	}
}

void logger::trim() {
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
