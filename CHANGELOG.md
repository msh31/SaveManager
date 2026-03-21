# Changelog

## [1.2.0] - 2026-03-19
### Added
- Remote sync over SFTP, to and from a server (With progress indicators & support for multi file transfers)
- Remove backup functionality
- Refresh saves button
- Open Path button (Opens the path to the detected savegame's location)

### Fixed 
- Silent JSON translation failure errors on startup [#3](https://github.com/msh31/SaveManager/issues/3)
- About tab showing outdated info from 1.0.0
- Memory leak caused by not deleting the in-memory textures on shutdown
- Curl url type causing segfaults under certain conditions

### Changes 
- Button presses now have some visual feedback
- General code refactoring in: ZipArchive, Detection & individual tabs

### Known Issues / Limitations
- Incorrect Game name (shown by steam appid or N/A) on some games that are not yet fully supported (Backups will not work properly since the names are similar if N/A) [#2](https://github.com/msh31/SaveManager/issues/2)
- Game Images aren't updated on refresh [#4](https://github.com/msh31/SaveManager/issues/4)
- Downloading of game images is not done asynchronously [#5](https://github.com/msh31/SaveManager/issues/5)

## [1.1.1] - 2026-03-14
### Fixed
- Unreal toggle not being wired up correctly

## [1.1.0] - 2026-03-14
### Added
- Heroic Games Launcher support (Ubisoft, Rockstar, Epic/Unreal)
- Unreal Engine game detection via .sav files (Linux Only)
- Update Translations button in settings

### Fixed
- getenv null check in paths
- zip_fread error handling
- temporary logger() instance in network.cpp

### Changes
- User Interface is more dynamic and polished to handle weird scenarios.
- Translations refactor (single init, O(1) lookups)

### Known Issues
- Incorrect Game name (shown by steam appid or N/A) on some games that are not yet supported (Backups will not work properly since the names are similar if N/A) (#2)

## [1.0.0] - 2026-03-10
### Added
- Steam/Proton save detection
- Lutris save detection  
- Ubisoft Connect support (Linux + Windows)
- Rockstar Games Launcher support (Linux + Windows)
- Save backup and restore
- Game cover art via Steam CDN
- Settings tab with configurable paths
