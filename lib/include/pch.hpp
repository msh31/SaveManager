#pragma once

#ifdef _WIN32
#define NOMINMAX        // prefer std::min/max over Windows macros
#define WIN32_LEAN_AND_MEAN // skip unused headers windows.h would otherwise drag in
#endif

#ifdef __cplusplus
#include <string>
#include <string_view>
#include <filesystem>
#include <vector>
#include <unordered_map>
#include <optional>
#include <set>
#include <chrono>
#include <fstream>
#include <algorithm>
#include <future>
#include <regex>
#include <optional>
#include <unordered_set>
#include <expected>
#include <ranges>
#include <cstring>
#include <deque>
#include <shared_mutex>
#include <print>
#include <memory>

#include <spdlog/spdlog.h>
#include <logger/logger.hpp>  
#include <spdlog/sinks/daily_file_sink.h>
#endif

#ifdef __linux__
#include <unistd.h>
#endif

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#endif
