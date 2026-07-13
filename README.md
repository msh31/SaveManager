# SaveManager

<p align="left">
  <img src="https://img.shields.io/badge/Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white" alt="Windows"/>
  <img src="https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black" alt="Linux"/>
  <img src="https://img.shields.io/badge/macOS-000000?style=for-the-badge&logo=apple&logoColor=white" alt="macOS"/>
</p>

[![Build](https://github.com/msh31/SaveManager/actions/workflows/build-test.yml/badge.svg?branch=dev)](https://github.com/msh31/SaveManager/actions/workflows/build-test.yml)
[![Release](https://github.com/msh31/SaveManager/actions/workflows/build-release.yml/badge.svg?branch=main)](https://github.com/msh31/SaveManager/actions/workflows/build-release.yml)
[![Version](https://img.shields.io/github/v/release/msh31/SaveManager)](https://github.com/msh31/SaveManager/releases)
---

## What is SaveManager?

A local and cross-platform save game manager with a built-in save-editor, sftp transfers and much more

**Key Features:**
- **Lua plugin** system for custom game detections (found [here](https://github.com/msh31/savemanager-plugins))
- **Experimental Save Editor** (only supports GTA San Andreas (classic PC))
- **Create and restore backups** of (individual) save files with display labels
- **Automatic detection** that finds your savegames (Steam, Heroic, Lutris, native and Wine!)
<!-- - **Scheduled backups**: Per-game schdedule to create a backup every X hours (Needs the optional daemon installed (linux only)) -->
<!-- - **Smart backup deletion**: configurable retention policy for old backups (Also needs the daemon) -->

---

## Supported 
for a full list of the supported games view [SUPPORTED_GAMES.md](docs/SUPPORTED_GAMES.md)

| Store / Savetype | Windows | Linux | macOS |
|------------------|---------|-------|-------|
| Ubisoft | Yes | Yes (Steam/Proton, Wine, Lutris, Heroic) | In development |
| Rockstar | Yes | Yes (Steam/Proton, Lutris, Heroic) | In development |
| Unreal Engine | Yes | Yes (Steam/Proton, Lutris, Heroic) | Yes |
| Minecraft | Yes | Yes | Yes | 

> [!NOTE]  
> Unreal Games are detected via GVAS .sav files. UE3 games and games using custom save formats are not supported. Only UE4/5

> [!NOTE] 
> Minecraft: the following stores are supported: Official, Modrinth, Curseforge, PrismLauncher (MultiMC is exclusive to linux)

---

## Preview



<details>
<summary>Screenshots</summary>

![Dark 1](/assets/screenshots/dark-1.png)
![Dark 2](/assets/screenshots/dark-2.png)
![Dark 3](/assets/screenshots/dark-3.png)

![Light 1](/assets/screenshots/light-1.png)
![Light 2](/assets/screenshots/light-2.png)
![Light 3](/assets/screenshots/light-3.png)

</details>

---



## Changelog
See [CHANGELOG.md](CHANGELOG.md) for a complete history of all changes.

### Planned Features

- **Scheduled backups**: Per-game schdedule to create a backup every X hours (Needs the optional daemon installed (linux only))
- **Smart backup deletion**: configurable retention policy for old backups (Also needs the daemon)
- **Save Editor**: expanding to Rockstar titles (RDR2, GTAV etc..)
- **PSP ↔ PPSSPP**: convert and transfer saves between physical PSP and emulator
- **Broader store support**: Crossover on MacOS, EA, Epic, Xbox App / MS Store etc..

### Download
You can download the latest ``.exe`` / ``.AppImage`` / ``.app`` file from here: \
<a href="https://github.com/msh31/SaveManager/releases">
    <img
        src="https://img.shields.io/badge/Download-C95D38?style=for-the-badge&labelColor=0C0D11"
        alt="Download"
        style="height: 50px"
    />
</a>

## Contributing

See [CONTRIBUTING.md](docs/CONTRIBUTING.md) for build instructions and contribution guidelines.

## License

GPLv3 see [LICENSE](LICENSE.md).

## Star History

[![Star History Chart](https://api.star-history.com/chart?repos=msh31/SaveManager&type=date&legend=top-left&sealed_token=zwvsFlaiThXfXZT4cr76r594q8v3uETEceXmNTQ1B3baSzonxatsb6bBtH6grMuCdPWWF_bJwfsVlPXBlx_4Hhxi5T3_aby-ppb4iTQLilIkGtv3-S4W_4l_1F8u_jgHP0e9AV6yyaC9wazdERQ4HkmVlL4to14hurvXC42ff5r-qVZXRhCg02f5NME6)](https://www.star-history.com/?repos=msh31%2FSaveManager&type=date&legend=top-left)
