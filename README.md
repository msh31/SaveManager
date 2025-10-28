# SaveManager
A local alternative to Ubisoft's Cloudsync for their savegames.

![Status](https://img.shields.io/badge/status-alpha-orange)
![Development](https://img.shields.io/badge/status-in%20development-yellow)
![Feedback](https://img.shields.io/badge/feedback-welcome-brightgreen)

---

## Motivation
Ubisoft's cloud sync in my opinion, sucks. I've had tons of save corruptions, entire wipe-outs you name it.
The project began around June 2024, but I never managed to actually finish it properly.
The original C# CLI version, which I wanted to replace with a GUI version in avalonia but I've come to love C++, ImGui and OpenGL and decided that I should do it properly.

## Features
- Local save backup and restore for Ubisoft games
- Cloud sync (ironic, but we use rsync like gigachads)
- Automatic detection of save directories
- Dated backups
- Minimal dependencies
- Simple & customizable GUI


## Building
### Requirements
- C++ 17+ compatible compiler (MSVC, GCC, or Clang)
- CMake 3.20 or higher

### Building
```zsh
git clone --recurse-submodules https://github.com/msh31/SaveManager.git
cd SaveManager

git submodule update --init --recursive

cmake -S . -B build
cmake --build build
# optional, built as release:
cmake --build build --config Release
```

## Dependencies
- [ImGui](https://github.com/ocornut/imgui) | Bloat-free Graphical User interface for C++ with minimal dependencies
- [GLFW](https://github.com/glfw/glfw) | A multi-platform library for OpenGL, OpenGL ES, Vulkan, window and input
- [Sentinel](https://github.com/msh31/sentinel) | A security SDK with modular architecture, built by me. It's mainly for Windows but the logger is cross-platform since it's just files.
> Note: After cloning, run: ``git submodule update --init --recursive``

<!--## Contributing
See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines on contributing to this project.
This includes opening issues, not just pull-requests.-->
