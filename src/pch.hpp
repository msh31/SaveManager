#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM //use glad not their own loader

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

#include "globals.hpp"

#include <imgui.h>
#include <imgui_stdlib.h>
#endif

#ifdef __linux__
#include <unistd.h>
#endif

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <shellapi.h>
#endif

#include <KHR/khrplatform.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
