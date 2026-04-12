# Contributing to SaveManager
Contributions are welcome. If you want to report a bug, suggest a feature, or submit a pull request, this document covers what you need to know.

## Reporting Issues
* Check existing issues before opening a new one
* Include your OS, how you installed the game (Steam, Lutris, Heroic, native), and steps to reproduce

## Pull Requests
* Keep PRs focused, one thing at a time
* If you're adding a feature, open an issue first to discuss it
* Follow the existing code style (C++23 for example)

---

## Building from Source

### Prerequisites

* [CMake](https://cmake.org/) 3.23 or later
* [Conan 2.x](https://conan.io/) (`pip install conan`)
* A C++23-capable compiler (GCC 13+, Clang 16+, MSVC 2022+)

All dependencies are managed by Conan, no manual installs needed.

> Note: Update submodules before building:
> ```
> git submodule update --init --recursive
> ```

---

> Change `conan-release` to `conan-debug` for debug builds.

### Linux

```bash
conan install . --build=missing
cmake --preset conan-release
cmake --build build/Release
```

Binary at `build/Release/savemanager`.

---

### macOS

```bash
conan install . --build=missing
cmake --preset conan-release
cmake --build build/Release
```

Binary at `build/Release/savemanager`.

> Full Disk Access may be required for save detection. Grant it under System Settings → Privacy & Security → Full Disk Access.

---

### Windows

Requires **Visual Studio 2022 or later** with the **Desktop development with C++** workload installed.

All commands must be run from a **Developer Command Prompt for VS** (or Developer PowerShell). This sets up the MSVC environment variables required by the build system. Find it in the Start menu under Visual Studio.

```bat
conan install . --output-folder=build --build=missing
cmake . -B build/out -G Ninja -DCMAKE_TOOLCHAIN_FILE="build/build/Release/generators/conan_toolchain.cmake" -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="build/build/Release/generators"
cmake --build build/out
```

Binary at `build/out/savemanager.exe`.

---

## Dependency Overview

All managed by Conan:

| Library | Purpose |
|-|-|
| [Dear ImGui](https://github.com/ocornut/imgui) | GUI |
| [GLFW](https://www.glfw.org/) | Window/input |
| [glad](https://github.com/Dav1dde/glad) | OpenGL loader |
| [nlohmann/json](https://github.com/nlohmann/json) | JSON parsing |
| [libcurl](https://curl.se/) | Networking |
| [libssh2](https://www.libssh2.org/) | SFTP transfer |
| [libzip](https://libzip.org/) | Archive handling |
| [stb](https://github.com/nothings/stb) | Image loading |
| [OpenSSL](https://www.openssl.org/) | SSL/TLS |

Git submodules
| Library | Purpose |
|-|-|
| [ImGuiFileDialog](https://github.com/aiekick/ImGuiFileDialog) | ImGui native file dialog for opening saves |

> Game art and translation data is fetched from GitHub on first launch and cached locally.
