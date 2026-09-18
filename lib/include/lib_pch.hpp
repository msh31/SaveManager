#pragma once
// Generic, stable C++ stdlib headers only

// clang-format off
#ifdef __cplusplus
    #include <algorithm>
    #include <atomic>
    #include <chrono>
    #include <cstring>
    #include <deque>
    #include <expected>
    #include <filesystem>
    #include <fstream>
    #include <functional>
    #include <future>
    #include <map>
    #include <memory>
    #include <optional>
    #include <mutex>
    #include <print>
    #include <ranges>
    #include <regex>
    #include <set>
    #include <shared_mutex>
    #include <stdexcept>
    #include <string>
    #include <string_view>
    #include <thread>
    #include <unordered_map>
    #include <unordered_set>
    #include <vector>

namespace fs = std::filesystem;
#endif

// TODO: undo this mess
#ifdef __linux__
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <unistd.h>
#endif

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <Windows.h>
    #include <Psapi.h>
    #include <ShlObj_core.h>
#endif
//clang-format on