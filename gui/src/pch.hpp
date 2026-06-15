#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM // use glad not their own loader

#ifdef __cplusplus
    #include <imgui.h>
    #include <imgui_internal.h>
    #include <imgui_stdlib.h>
#endif

// clang-format off
#include <KHR/khrplatform.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
// clang-format on
