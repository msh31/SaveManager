# SaveManager
SaveManager is a cross-platform game save manager with automatic save detection, a built-in save editor, and SFTP transfers.

[![Dev Build](https://github.com/msh31/SaveManager/actions/workflows/build-test.yml/badge.svg?branch=dev)](https://github.com/msh31/SaveManager/actions/workflows/build-test.yml)
[![Version](https://img.shields.io/github/v/release/msh31/SaveManager)](https://github.com/msh31/SaveManager/releases)
---

![Pretend there is an app screenshot here](/assets/version190.png)

## Features
- **Save Editor** - Edit your save data (only supports GTA San Andreas (classic PC))
- **Automatic detection** - Finds your savegames without user interaction (Across stores like Steam, Heroic, Lutris, Epic, Xbox/MS Store, EA)
- **Mass Backup** - Easily create backups of all your savefiles in one shot!
- **Blacklist** - Prevent some games from showing in detection results (Online only games for example)
<!-- - **Scheduled backups**: Per-game schdedule to create a backup every X hours (Needs the optional daemon installed (linux only)) -->
<!-- - **Smart backup retention**: configurable policy to keep X old backups -->

#### Planned
- Scheduled backups: Schedule a backup for a game, on a per save basis or all saves
- Smart backup retention: Automatically delete old backups after X days or keep only X copies of 1 backup
- Save Editor expansion: GTAV, RDR2 to start off with
- Emulator Support: RPCS3, PPSSPP, Dolphin, etc..
- PSP <--> PPSSPP: convert and transfer saves between physical PSP and emulator
- Broader store support: Crossover on MacOS

---

## Downloads
> [!NOTE]
> SaveManager is a hobby project of mine, so unexpected issues may occur.
> Please report them if they haven't already been reported.

Download the portable executable for Windows, Linux, or Mac from [here](https://github.com/msh31/SaveManager/releases)

### Note
- Windows users may see a popup that says "Windows protected your PC" because Windows does not recognize the program's publisher. Click "more info" and then "run anyway" to start the program.
- Mac users may see a popup that says "SaveManager can't be opened because it is from an unidentified developer". Go to Settings -> Privacy & Security and click "Allow" on SaveManager

---

## Supported 
For a full list of the supported games view the [SUPPORTED_GAMES](docs/SUPPORTED_GAMES.md) file

| Type | Windows | Linux | macOS |
|------------------|---------|-------|-------|
| Ubisoft | Yes | Yes (Steam/Proton, Lutris, Heroic) | X |
| Rockstar | Yes | Yes (Steam/Proton, Lutris, Heroic) | X |
| Unreal Engine | Yes | Yes (Steam/Proton, Lutris, Heroic) | Yes |
| Minecraft | Yes | Yes | Yes | 

> [!NOTE]  
> Unreal Games are detected via GVAS .sav files. UE3 games and games using custom save formats are not supported. Only UE4/5

> [!NOTE] 
> Minecraft: the following stores are supported: Official, Modrinth, Curseforge, PrismLauncher (MultiMC is exclusive to linux)

---

## Contributing
See [CONTRIBUTING](docs/CONTRIBUTING.md) for build instructions and contribution guidelines.

## License
GPLv3 see [LICENSE](LICENSE.md).

## Star History

<a href="https://www.star-history.com/?repos=msh31%2FSaveManager&type=date&legend=top-left">
 <picture>
   <source media="(prefers-color-scheme: dark)" srcset="https://api.star-history.com/chart?repos=msh31/SaveManager&type=date&theme=dark&legend=top-left&sealed_token=O4YTlWsvOU4koHndgz1fcB6Lz2IS70_zMJew4qWKO2fYrKWOLiqtQnq5Wclj92Y4LIIv_wQG4T4Ah_0vo5-_kTzNLKYZ13UlxLh6HIwA9ATgNUqUBbygQvj0-NvAgi3jy3QiPthBmPyA_WsTxMrpZ-nnBJSlzasnRnKMl7iNGtkArDTbYSwWyOJMomm5" />
   <source media="(prefers-color-scheme: light)" srcset="https://api.star-history.com/chart?repos=msh31/SaveManager&type=date&legend=top-left&sealed_token=O4YTlWsvOU4koHndgz1fcB6Lz2IS70_zMJew4qWKO2fYrKWOLiqtQnq5Wclj92Y4LIIv_wQG4T4Ah_0vo5-_kTzNLKYZ13UlxLh6HIwA9ATgNUqUBbygQvj0-NvAgi3jy3QiPthBmPyA_WsTxMrpZ-nnBJSlzasnRnKMl7iNGtkArDTbYSwWyOJMomm5" />
   <img alt="Star History Chart" src="https://api.star-history.com/chart?repos=msh31/SaveManager&type=date&legend=top-left&sealed_token=O4YTlWsvOU4koHndgz1fcB6Lz2IS70_zMJew4qWKO2fYrKWOLiqtQnq5Wclj92Y4LIIv_wQG4T4Ah_0vo5-_kTzNLKYZ13UlxLh6HIwA9ATgNUqUBbygQvj0-NvAgi3jy3QiPthBmPyA_WsTxMrpZ-nnBJSlzasnRnKMl7iNGtkArDTbYSwWyOJMomm5" />
 </picture>
</a>
