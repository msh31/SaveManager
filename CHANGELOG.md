# Changelog

## [1.5.0] - 2026-04-04
### New verified supported games

### Added
- MacOS support (Intel & Apple Silicon)

### Fixed 


### Changes
- Code improvements for better maintainability
- Use C++23 throughout the whole codebase
- Use VCPKG for external dependencies

### Known Issues / Limitations


## [1.4.1] - 2026-04-04
### Fixed
- Unreal cover art / showing up as N/A on Linux

## [1.4.0] - 2026-04-04
### New verified supported games
- Mafia The Old Country
- Clair Obscur: Expedition 33
- Metal Gear Solid Delta: Snake Eater
- Nobody Wants To Die
- High on Life
- SpongeBob SquarePants: Titans of the Tide
- Assassin's Creed Mirage 
- Assassin's Creed Shadows 

### Added
- Unreal support on Windows
- Mass Backup button 
- Refresh cache button 
- Backup labels
- Dark / Light Mode toggle
- Copy log to clipboard
- Automatic log cleanup (If above 100 lines)

### Fixed
- Duplicate cards for games with N/A appid [#7](https://github.com/msh31/SaveManager/issues/7)
- Incorrect Game name (shown by steam appid or N/A) on some games that are not yet supported (Backups will not work properly since the names are similar if N/A) [#2](https://github.com/msh31/SaveManager/issues/2)
- A crash caused by attempting to load corrupted translation files.

### Changes
- An Internet connection is no longer required by default 
- Codebase changes in the entrypoint
- Overall UI improvements for better User Experience
- Improved overall detection accuracy
- Use %USERPROFILE% (C:\Users\username) as home dir instead of appdata for Windows
- Codebase changes in the entrypoint and other areas


### Known Issues / Limitations
- Original Anno editions not supported due to install path limitations (You can add them manually though!)
- Grouping issues with unknown / partially supported games (Similar to #2 but has since improved)


## [1.3.0] - 2026-03-26
### New Game support
- The original GTA Trilogy 
- Anno franchise
- Manhunt 1 & 2
- Max Payne 1 & 2
- God of War 2018 & Ragnarok
- Crimson Desert (Beta, through custom games)

### Added
- Check for updates button
- Game blacklist (User configurable)
- Custom Game list (User configurable)
> Note: both the blacklist and custom list do also get downloaded from github
    The custom game list is there for non specific game launcher / publisher games
    and it will receive more updates, hopefully with community additions

### Fixed 
- Game Images aren't updated on refresh [#4](https://github.com/msh31/SaveManager/issues/4)
- Downloading of game images is not done asynchronously [#5](https://github.com/msh31/SaveManager/issues/5) (semi-fixed)

### Changes 
- Improved support for Unreal Games
- Texture loading is now done asynchronously
- Logger is now thread-safe
- No more direct system calls to open a path

### Known Issues / Limitations 
- Duplicate cards for games with N/A appid [#7](https://github.com/msh31/SaveManager/issues/7)
- Incorrect Game name (shown by steam appid or N/A) on some games that are not yet supported (Backups will not work properly since the names are similar if N/A) [#2](https://github.com/msh31/SaveManager/issues/2)

- Original Anno editions not supported due to install path limitations (You can add them manually though!)


## [1.2.1] - 2026-03-21
### Fixed
- Password authentication being the only option for SFTP transfers
- A path joining bug in the SFTP implementation
- Incorrect path passed to the upload and download buttons for SFTP transfers
- Connect button not being hidden when connected
- Out of bounds error in the transfer tab


## [1.2.0] - 2026-03-21
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
