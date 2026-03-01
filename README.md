# SaveManager
A lightweight and efficient savegame manager to help you manage your game saves with ease.

<p>
<img src="https://img.shields.io/badge/Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white" alt="Windows"/>
<img src="https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black" alt="Linux"/>
</p>

---

## Under Construction
This project is still under development, and features may be subject to change.

## Features (working)
- **Backup and Restore**
  Create backups of your savegames and restore them when needed, with custom name support!

### Planned Features
- **Web sync**
  Sync save games to your savehub account (seperate wip project)

## Supported Platforms
- Ubisoft
- Rockstar
- Unreal (Planned)
- PSP / PPSSPP (Planned)

## Building

After cloning, make sure to grab the submodules:
```bash
git submodule update --init --recursive
```

### Dependencies

Bundled in `external/` (no extra install needed):
- [nlohmann/json](https://github.com/nlohmann/json)
- [glad](https://github.com/Dav1dde/glad) (OpenGL loader)
- [Dear ImGui](https://github.com/ocornut/imgui)
- [stb](https://github.com/nothings/stb)

System dependencies you need to install:
- [glfw3](https://github.com/glfw/glfw)
- [libzip](https://github.com/winlibs/libzip)
- [curl](https://github.com/curl/curl)

Also fetched at runtime:
- [ubisoft_game_ids](https://git.marco007.dev/marco/Ubisoft-Game-Ids) — downloaded once on first launch

### Linux

Install the system dependencies, then build with cmake:

```bash
# Arch
sudo pacman -S glfw-x11 libzip curl

# Debian/Ubuntu
sudo apt install libglfw3-dev libzip-dev libcurl4-openssl-dev

# Fedora
sudo dnf install glfw-devel libzip-devel libcurl-devel
```

```bash
cmake -B build
cmake --build build -j$(nproc)
```

The binary will be at `build/savemanager`.

### Windows (Visual Studio)

This uses vcpkg (bundled with VS 2026+) to pull in glfw3, libzip, and curl automatically.

Edit `configure-vs.bat` if your VS install path or vcpkg location differs, then:

```bat
configure-vs.bat
```

This generates a VS solution in `build-vs/`. Open it and build from there.

If you have a standalone vcpkg install instead, just point `CMAKE_TOOLCHAIN_FILE` at it:
```bat
cmake -B build -G "Visual Studio 18 2026" -A x64 ^
  -DCMAKE_TOOLCHAIN_FILE="path\to\vcpkg\scripts\buildsystems\vcpkg.cmake"
```

### Cross-compiling for Windows (from Linux)

You can build a Windows `.exe` from Linux using MinGW-w64 and vcpkg:

1. Install MinGW-w64:
```bash
# Arch
sudo pacman -S mingw-w64-gcc
```

2. Set up vcpkg if you don't have it already:
```bash
git clone https://github.com/microsoft/vcpkg.git ~/vcpkg
cd ~/vcpkg && ./bootstrap-vcpkg.sh
export VCPKG_ROOT="$HOME/vcpkg"  # add this to your shell rc
```

3. Run the build script:
```bash
./build-windows.sh
```

The script will use vcpkg to pull in the Windows versions of glfw3, libzip, and curl, then cross-compile everything. The resulting executable lands at `build-windows/savemanager.exe`.
