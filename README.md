# SaveManager
A lightweight and efficient savegame manager to help you manage your game saves with ease.

## Under Construction
This project is still under development, and features may be subject to change. 

## Features
- **Backup and Restore Saves**  
  Create backups of your savegames and restore them when needed.
  
- **List Saves and Backups**  
  View all available saves and their backups with a simple command.

## Platforms
- Ubisoft
- Rockstar (Planned)
- Unreal (Planned)
- PSP / PPSSPP (Planned)

### Dependencies
1. [nlohmann/json](https://github.com/nlohmann/json) - Working with json files
2. [ubisoft_game_ids](https://git.marco007.dev/marco/Ubisoft-Game-Ids) - Ubisoft gameID translations
3. [lipzip](https://github.com/winlibs/libzip) - Creating zip archives crossplatform

```bash
# INFO
# make sure to install libzip on your system to be able to build this project!

#arch
sudo pacman -S libzip

#MacOS
brew install libzip
