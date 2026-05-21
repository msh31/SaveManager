#pragma once

#ifdef _WIN32
#define NOMINMAX            // prefer std::min/max over Windows macros
#define WIN32_LEAN_AND_MEAN // skip unused headers windows.h would otherwise drag in

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#ifdef __cplusplus
#include <algorithm>
#include <chrono>
#include <cstring>
#include <deque>
#include <expected>
#include <filesystem>
#include <fstream>
#include <future>
#include <memory>
#include <optional>
#include <print>
#include <ranges>
#include <regex>
#include <set>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <logger/logger.hpp>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/spdlog.h>
#endif

#ifdef __linux__
#include <unistd.h>
#endif
