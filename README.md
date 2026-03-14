<div align="center">

# SaveManager
### The swiss army knife of save management

<img src="https://img.shields.io/badge/Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white" alt="Windows"/>
<img src="https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black" alt="Linux"/>
<img src="https://img.shields.io/badge/license-GPLv3-green?style=for-the-badge" alt="GPLv3 License"/>

[Download](#download) • [Features](#features) • [Contributing](CONTRIBUTING.md) • [License](#license)

<img src="assets/output_hq.gif" alt="App Preview" width="800"/>

</div>

---

SaveManager is a local save manager for PC games. It handles backup and restore for Ubisoft and Rockstar saves across Windows and Linux.

## Download

Available for Windows and Linux (x86-64) on the [releases page](https://github.com/msh31/SaveManager/releases).

## Features

- **Backup & Restore** — create and restore save backups
- **Automatic Detection** — finds Ubisoft and Rockstar saves on Windows; Steam/Proton, Wine, and Lutris on Linux
- **Configurable** — custom backup paths, per-store toggles, and platform-specific settings
- **Cross-platform** — Windows and Linux (x86-64)

### Planned

- **Save Editor** — read and edit save data, starting with Rockstar titles
- **PSP ↔ PPSSPP** — convert and transfer saves between physical PSP and emulator
- **Web Sync** — sync and share saves via an upcoming web platform

## Supported Stores

| Store / Savetype | Windows | Linux |
|-------|---------|-------|
| Ubisoft | Yes | Yes (Steam/Proton, Wine, Lutris, Heroic) |
| Rockstar | Yes | Yes (Steam/Proton, Lutris, Heroic) |
| Unreal Engine | Planned | Yes (Steam/Proton, Lutris, Heroic) |
> Unreal Engine games are detected via .sav files, regardless of store. (It is also not perfect)
---

## Changelog
See [CHANGELOG.md](CHANGELOG.md) for a complete history of all changes made to the project.

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for build instructions and contribution guidelines.

## License

GPLv3 — see [LICENSE](LICENSE.md).

## Star History

<a href="https://www.star-history.com/?repos=msh31%2FSaveManager&type=date&legend=top-left">
 <picture>
   <source media="(prefers-color-scheme: dark)" srcset="https://api.star-history.com/image?repos=msh31/SaveManager&type=date&theme=dark&legend=top-left" />
   <source media="(prefers-color-scheme: light)" srcset="https://api.star-history.com/image?repos=msh31/SaveManager&type=date&legend=top-left" />
   <img alt="Star History Chart" src="https://api.star-history.com/image?repos=msh31/SaveManager&type=date&legend=top-left" />
 </picture>
</a>
