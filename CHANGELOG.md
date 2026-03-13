# Changelog

## [1.1.0] - 2026-03-14
### Added
- Heroic Games Launcher support (Ubisoft, Rockstar, Epic/Unreal)
- Unreal Engine game detection via .sav files (Linux Only)

### Fixed
- getenv null check in paths
- zip_fread error handling
- temporary logger() instance in network.cpp

### Changes
- User Interface is more dynamic and polished to handle weird scenarios.
- Translations refactor (single init, O(1) lookups)

## [1.0.0] - 2026-03-10
### Added
- Steam/Proton save detection
- Lutris save detection  
- Ubisoft Connect support (Linux + Windows)
- Rockstar Games Launcher support (Linux + Windows)
- Save backup and restore
- Game cover art via Steam CDN
- Settings tab with configurable paths
