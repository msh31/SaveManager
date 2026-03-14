# Contributing to SaveManager

Contributions are welcome. If you want to report a bug, suggest a feature, or submit a pull request; this document covers what you need to know.

## Reporting Issues

- Check existing issues before opening a new one
- Include your OS, how you installed the game (Steam, Lutris, native), and steps to reproduce

## Pull Requests

- Keep PRs focused, one thing at a time
- If you're adding a feature, open an issue first to discuss it
- Follow the existing code style (C++17, namespace over heavy OOP)

---

## Building from Source
After cloning, grab the submodules:

```bash
git submodule update --init --recursive
```

### Dependencies
Bundled in `external/` (no install needed):

| Library | Purpose |
|---------|---------|
| [nlohmann/json](https://github.com/nlohmann/json) | JSON parsing |
| [glad](https://github.com/Dav1dde/glad) | OpenGL loader |
| [Dear ImGui](https://github.com/ocornut/imgui) | GUI |
| [stb](https://github.com/nothings/stb) | Image loading |

System dependencies (glfw3, libzip, curl) are managed by vcpkg and installed automatically during the build.

> Game ID data is fetched from GitHub on first launch and cached locally.

The build configuration lives in `cmake.toml` ([cmkr](https://github.com/build-cpp/cmkr)), which auto-generates `CMakeLists.txt` — edit `cmake.toml`, not `CMakeLists.txt` directly.

### Linux

```bash
cmake -B build
cmake --build build -j$(nproc)
```
> Note: use CMake 3.31.x as CMake 4.x.x does not work on some dependencies!

Binary at `build/savemanager`.

### Windows (Visual Studio)
vcpkg (bundled with VS) pulls in glfw3, libzip, and curl automatically same as linux.

Edit `configure-vs.bat` if your VS install path, VS version or vcpkg location differs, then:

```bat
configure-vs.bat
```

This generates a VS solution in `build-vs/`. Open it and build from there.

If you have a standalone vcpkg install (make sure to change the version to yours):

```bat
cmake -B build -G "Visual Studio 17 2022" -A x64 ^
  -DCMAKE_TOOLCHAIN_FILE="path\to\vcpkg\scripts\buildsystems\vcpkg.cmake"
```
