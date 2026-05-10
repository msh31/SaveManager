#pragma once
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
#endif

#ifdef __linux__
#include <unistd.h>
#endif

#ifdef _WIN32
#define NOMINMAX
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <shellapi.h>
#include <wchar.h>
#include <KnownFolders.h>
#include <shlobj.h>
#endif
