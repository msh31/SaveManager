@echo off

set VCPKG_ROOT=C:\Program Files\Microsoft Visual Studio\17\Community\VC\vcpkg

cmake -B build-vs ^
  -G "Visual Studio 17 2022" ^
  -A x64 ^
  -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" ^
  -DVCPKG_TARGET_TRIPLET=x64-windows-static ^
  -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded
