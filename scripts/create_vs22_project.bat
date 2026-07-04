conan install . --build=missing -s build_type=Debug
cmake --preset conan-debug -G "Visual Studio 17 2022"