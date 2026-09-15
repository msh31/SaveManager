# Contributing
Thank you for your interest in contributing!

If you want to report a bug, suggest a feature, or submit a pull request, this document covers what you need to know.

## Reporting Issues
* Check existing issues before opening a new one
* Include the following information:
	- Operating System 
	- SaveManager version
	- A short description of the problem
	- Screenshots (if applicable)
	- Latest log file
	- Steps to reproduce

## Pull Requests
* Keep PRs focused, one thing at a time
* If you're adding a feature, open an issue first to discuss it
* Follow the existing code style

---

## Plugin development

SaveManager supports Lua plugins for community-contributed game definitions. If you want to add support for a game without modifying the core codebase, write a plugin instead.

See [SCRIPTING.md](SCRIPTING.md) for the full API reference, and submit your plugin to the [savemanager-plugins](https://github.com/msh31/savemanager-plugins) repository.

---

## A.I. Rules
- Use of A.I. coding tools **must** be disclosed and described. If you are found to have used A.I. assistance without disclosure, you may be banned from making further PRs. (You can do this by making sure the commit/PR is co-authored by the A.I. assistent)
- Descriptions and comments should never be written by an A.I., everything needs to be human written, so it is human readable.
- Any A.I-generated code that you wish to keep in the PR needs to be pre-reviewed and polished. The user needs to be able to logically explain their decisions, including those of any A.I. they use, regardless of the code's functionality.

---

## Style

### Formatting
A ``clang-format`` file is used to format the codebase so that you can focus on writing code

### Naming
When writing new classes or interfaces etc make sure that you conform to the norm of prefixing it like so:

- Classes & Interfaces: Prefix them with the letter those words start with like so: ``CDetection`` ``ISomeInterface``
- Variables: prefix them depending on their scope like so: ``m_member_fn`` ``g_global_fn``

---

## Building from Source

### Prerequisites

* [CMake](https://cmake.org/) 3.23 or later
* [Ninja](https://ninja-build.org/)
* [Conan 2.x](https://conan.io/) (requires [Python](https://www.python.org/downloads/))
* A C++23-capable compiler (GCC 14+, Clang 16+, MSVC 2022+)

All dependencies are managed by Conan or are vendored directly in the vendor directory

---

### Build

```bash
conan install . --build=missing
cmake --preset conan-debug
cmake --build build/Debug
```

Binary at `build/Debug/savemanager`.

---

> MacOS users: Full Disk Access may be required for save detection. \
Grant it under ``System Settings → Privacy & Security → Full Disk Access``

---

### Windows

Requires **Visual Studio 2022 or later** with the **Desktop development with C++** workload installed.

All commands must be run from a **Developer Command Prompt for VS** (or Developer PowerShell). This sets up the MSVC environment variables required by the build system. Find it in the Start menu under Visual Studio.

```bat
conan install . --output-folder=build --build=missing
cmake --preset conan-default
cmake --build build/
```

Binary at `build/Debug/savemanager.exe`.

---

## Dependency Overview
Conan:

| Library | Purpose | Note
|-|-|-|
| [Dear ImGui](https://github.com/ocornut/imgui) | GUI | has some loose C++ wrappers in vendor
| [GLFW](https://www.glfw.org/) | Window/input |
| [glad](https://github.com/Dav1dde/glad) | OpenGL loader |
| [nlohmann/json](https://github.com/nlohmann/json) | JSON parsing |
| [libcurl](https://curl.se/) | Networking |
| [libssh2](https://www.libssh2.org/) | SFTP transfer |
| [libzip](https://libzip.org/) | Archive handling |
| [OpenSSL](https://www.openssl.org/) | SSL/TLS |
| [sol2](https://github.com/ThePhD/sol2) | Lua scripting |

Vendored directly:

| Library | Purpose |
|-|-|
| [nativefiledialog-extended](https://github.com/btzy/nativefiledialog-extended) | Native OS file picker |
| [stb](https://github.com/nothings/stb) | Image loading |
| [sha256](https://github.com/System-Glitch/SHA256) | SHA256 hashing

---

References for sections of this document:
- [shadPS4](https://github.com/shadps4-emu/shadPS4/) (using a modified version of their A.I. rules)