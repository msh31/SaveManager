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
  Create backups of your savegames and restore them when needed.
  
- **List Saves and Backups**  
  View all available saves and their backups with a simple command.

### Planned Features
- **Web sync**
    Sync save games to your savehub account (seperate wip project)
- **Custom Save Names**
    Give custom names to specific backups of savefiles e.x: "Hardcore-2026-02-02-16-00-00"
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
