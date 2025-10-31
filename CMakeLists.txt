cmake_minimum_required(VERSION 3.20)
project(savemanager)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/external/imgui)
set(GLFW_DIR ${CMAKE_SOURCE_DIR}/external/glfw)
set(SENTINEL_DIR ${CMAKE_SOURCE_DIR}/external/sentinel)

add_subdirectory(${GLFW_DIR})
add_subdirectory(${SENTINEL_DIR})

set(IMGUI_SRC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    ${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
)

add_library(savemanager_core STATIC
    src/core/backup/backup.cpp
    src/core/config/config.cpp
    src/core/detection/profile.cpp
    src/core/detection/games.cpp
    src/core/detection/saves.cpp
    src/core/utils/save_parser.cpp
)

target_include_directories(savemanager_core PUBLIC
    src/core
    ${SENTINEL_DIR}/include
)

target_link_libraries(savemanager_core
    sentinel
)

add_executable(savemanager
    src/gui/main.cpp
    src/gui/theme_manager/theme_manager.cpp
    src/gui/ui_manager/ui_manager.cpp
    ${IMGUI_SRC}
)

if(WIN32)
    set_target_properties(savemanager PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
endif()

target_include_directories(savemanager PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${IMGUI_DIR}/misc/cpp
    ${SENTINEL_DIR}/include
)

find_package(OpenGL REQUIRED)

target_link_libraries(savemanager
    savemanager_core
    glfw
    OpenGL::GL
)
