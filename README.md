# SaveManager
A lightweight and efficient savegame manager to help you manage your game saves with ease.

<p>
<img src="https://img.shields.io/badge/Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white" alt="Windows"/>
<img src="https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black" alt="MacOS"/>
<img src="https://img.shields.io/badge/MacOS-f0f0f0?logo=apple&logoColor=black&style=for-the-badge" alt="MacOS"/> 
</p>

---

## Under Construction
This project is still under development, and features may be subject to change. 

#### Where's the download?
Currently there is only some linux binaries, but you can easily compile this project for yourself like so;
```bash
cmake -B build
cmake --build build
```

## Features
- **Backup and Restore Saves**  
  Create backups of your savegames and restore them when needed, with custom name support!
  
- **List Saves and Backups**  
  View all available saves and their backups with a simple command.

### Planned Features
- **Web sync**  
  Sync save games to your savehub account (seperate wip project)
- **Better UX**  
  Current design works, but it could of course be better. Perhaps with a GUI.

## Support Platforms
- Ubisoft 
- Rockstar 
- Unreal (Planned)
- PSP / PPSSPP (Planned)

### Dependencies
1. [nlohmann/json](https://github.com/nlohmann/json) - Working with json files
2. [ubisoft_game_ids](https://git.marco007.dev/marco/Ubisoft-Game-Ids) - Ubisoft gameID translations | downloaded once on startup of the program
3. [libzip](https://github.com/winlibs/libzip) - Creating zip archives crossplatform

```bash
# INFO
# make sure to install libzip on your system to be able to build this project!

#arch
sudo pacman -S libzip

#MacOS
brew install libzip

#Windows (Using [vcpkg](https://vcpkg.io/en/index.html))
vcpkg integrate install
vcpkg install libzip
```


#### Cross-compiling for Windows (from Linux)
You can cross-compile Windows executables from Linux using MinGW-w64:

1. Install MinGW-w64:
```bash
# Arch, might differ on other distros
sudo pacman -S mingw-w64-gcc
```

2. Install vcpkg (for dependency management):
```bash
git clone https://github.com/microsoft/vcpkg.git ~/vcpkg
cd ~/vcpkg && ./bootstrap-vcpkg.sh
export VCPKG_ROOT="~/vcpkg" # add to zsh or bash or fish shell
```

3. Build using the provided script:
```bash
./build-windows.sh
```

Or manually (still needs vcpkg):
```bash
cmake -B build-windows -DCMAKE_TOOLCHAIN_FILE=toolchains/toolchain-mingw-w64-vcpkg.cmake
cmake --build build-windows
```

The Windows executable will be at `build-windows/savemanager.exe`

**Note:** vcpkg will automatically download and build libzip and curl for Windows on first build.
