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

* [CMake](https://cmake.org/) 3.20 or later
* [Conan 2.x](https://conan.io/) (`pip install conan`)
* A C++23-capable compiler (GCC 13+, Clang 16+, MSVC 2022)

All dependencies are managed by Conan, no manual installs needed.

---

> Just change conan-release to conan-debug for debug builds.

> Note: Make sure to update the submodules!
``git submodule update --init --recursive``

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

> Full Disk Access may be required for save detection. Grant it under System Settings → Privacy \& Security → Full Disk Access.

---

### Windows

#### Visual Studio 2022

Run the included script:

```bat
create\_vs22\_project.bat
```

This runs:

```bat
conan install . --build=missing -s build\_type=Debug
cmake --preset conan-default -G "Visual Studio 17 2022"
```

Then open the generated solution in `build/` and build from Visual Studio.

> If the preset name differs on your machine, check the available presets with `cmake --list-presets` after the Conan install step.

#### Command Line (Release)

```bat
conan install . --build=missing
cmake --preset conan-default
cmake --build build --config Release
```

---

## Dependency Overview

All managed by Conan:

|Library|Purpose|
|-|-|
|[Dear ImGui](https://github.com/ocornut/imgui)|GUI|
|[GLFW](https://www.glfw.org/)|Window/input|
|[glad](https://github.com/Dav1dde/glad)|OpenGL loader|
|[nlohmann/json](https://github.com/nlohmann/json)|JSON parsing|
|[libcurl](https://curl.se/)|Networking|
|[libssh2](https://www.libssh2.org/)|SFTP transfer|
|[libzip](https://libzip.org/)|Archive handling|
|[stb](https://github.com/nothings/stb)|Image loading|
|[OpenSSL](https://www.openssl.org/)|SSL/TLS|

> Game ID and translation data is fetched from GitHub on first launch and cached locally.

