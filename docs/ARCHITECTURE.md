# SaveManager's Architecture
This document aims to help potentional contributors understand the spaghetti I have cooked up

## General Information
- This is a [monorepo](https://en.wikipedia.org/wiki/Monorepo) and is split up in 4 main folders:
    - lib
        - This is the core library; it handles detection, creating / removing backups, logging and plugins.
    - gui
        - This is the GUI wrapper aroun the library, with multiple views and largely abstracted OpenGL logic,
    - cli
        - This is the CLI wrapper around the library, it can be used with flags for automation or interactively
    - daemon
        - This is the background service for handling tasks such as scheduling backups, cleaning up old ones etc..
        - It links the library and also exposes a socket for the GUI/CLI for inter process communication

- I use [cmkr](https://cmkr.build/) to build this proejct, you must edit [cmake.toml](../cmake.toml) and not the CMakeLists.txt file directly.

---

## Library
- Config | handles all config logic such as creation, saving changes, populating a pre-set blacklist 

- Detection | handles the scanning logic to find savegames and also loads [plugins](SCRIPTING.md):
    - CUbisoftDetector | Searches for saves in known paths for Ubisoft games
    - CRockstarDetector | Searches for saves in known paths for Rockstar games
    - CUnrealDetector | Searches for saves matching Unreal's identity, determined by extension & header
    - CWinePrefixDetector | Wraps the above mentioned detectors for Linux / MacOS operating systems
    - CMinecraftDetector | Searches for minecraft worlds in known paths for various launchers 

- Features (Core features of the savemanager-lib)
    - Backup | handles backup and restoration of savefiles
    - Remote Transfer | allows users to transfer files over SFTP 
    - Save Editor | Editor for GTA San Andreas PC savefiles

- Network | a helper utility to download files and checking for updates

- Plugin | Finds, loads and runs user created plugins from ``path/to/config/plugins``

- Utils
    - Blacklist | A pre-made blacklist that is usre editable through the config
    - Steam | Helper functions for interacting with steam
    - Translations | Handles Ubisoft gameid translations and other steam AppID translations
    - ZipArchive | Creates, Extracts archives with helper methods for a manifest json with backup info

## GUI 
## CLI 
## Daemon
