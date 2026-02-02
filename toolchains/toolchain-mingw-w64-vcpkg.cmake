set(VCPKG_TARGET_TRIPLET "x64-mingw-static" CACHE STRING "")
set(VCPKG_HOST_TRIPLET "x64-linux" CACHE STRING "")
set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/toolchain-mingw-w64-base.cmake" CACHE STRING "")

if(NOT DEFINED ENV{VCPKG_ROOT})
    message(FATAL_ERROR "VCPKG_ROOT environment variable not set")
endif()

include("$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
