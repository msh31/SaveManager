#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM // use glad not their own loader

// Generic stdlib comes from lib_pch.hpp (shared with template_lib, so tests
// don't need imgui). Only GUI-specific, still-stable headers belong here.
// Project headers you edit often (backend/logger.hpp, types.hpp) do NOT
// belong in either PCH - see lib_pch.hpp.
#include <lib_pch.hpp>

#ifdef __cplusplus
    #include "imgui.h"
    #include "imgui_stdlib.h"
#endif

// clang-format off
#include <glad/gl.h>
#include <KHR/khrplatform.h>
#include <GLFW/glfw3.h>
// clang-format on
