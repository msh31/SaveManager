# SaveManager

<p align="center">
  <img src="assets/logo.svg" alt="SaveManager Logo" style="width: 192px" />
  <br>
</p>

<p align="center">
    <!-- yes, github.... codeberg doesnt have windows or mac runners, and i cba setting them up myself rn -->
    <a href="https://github.com/msh31/SaveManager/releases">
        <img
            src="https://img.shields.io/badge/Download-C95D38?style=for-the-badge&labelColor=0C0D11"
            alt="Download"
            style="height: 50px"
        />
    </a>
</p>

<p><br/></p>

<p align="center">
  <a href="https://codeberg.org/marco007/SaveManager/commits/branch/main">
    <img src="https://img.shields.io/github/last-commit/msh31/SaveManager?style=for-the-badge&labelColor=1A1A18&color=C95D38&logo=git&logoColor=FFFFFF&label=commit" alt="Last commit" />
  </a>
  <a href="https://codeberg.org/marco007/SaveManager">
    <img src="https://img.shields.io/badge/Codeberg-2185D0?style=for-the-badge&logo=codeberg&logoColor=white" alt="Codeberg" />
  </a>
    <a href="https://github.com/msh31/SaveManager">
    <img src="https://img.shields.io/badge/GitHub-181717?style=for-the-badge&logo=github&logoColor=white" alt="Github" />
  </a>
</p>

<p><br /></p>

<p align="center">
      <img src="https://img.shields.io/badge/Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white" alt="Windows"/>
    <img src="https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black" alt="Linux"/>
    <img src="https://img.shields.io/badge/macOS-000000?style=for-the-badge&logo=apple&logoColor=white" alt="macOS"/>
</p>

---

## What is SaveManager?

A local and cross-platform save game manager for Ubisoft, Rockstar and Unreal Engine saves. Built with [ImGui](https://github.com/ocornut/imgui) and [OpenGL](https://www.opengl.org)

**Key Features:**
- Create and restore backups of individual save files with optional labels
- Automatic detection that finds your savegames (Steam, Heroic, Lutris, native and Wine!)
- Experimental Save Editor (only supports GTA San Andreas (classic PC))
- Custom backup paths, per-store toggles, game blacklist, and custom game definitions

---

## Preview

<img src="assets/output.gif" alt="App Preview"/>

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

## Supported Games
See [SUPPORTED_GAMES.md](docs/SUPPORTED_GAMES.md)

### Planned Features

- **Save Editor** — expanding to Rockstar titles (RDR2); GTA SA is experimental and current
- **PSP ↔ PPSSPP** — convert and transfer saves between physical PSP and emulator
- **Smart backup deletion** — configurable retention policy for old backups
- **Backup automation** — headless background process for scheduled backups
- **Broader game support** — Minecraft, Unity games (Mouse: P.I. For Hire), and more

## Supported Stores

| Store / Savetype | Windows | Linux | macOS |
|------------------|---------|-------|-------|
| Ubisoft | Yes | Yes (Steam/Proton, Wine, Lutris, Heroic) | In development |
| Rockstar | Yes | Yes (Steam/Proton, Lutris, Heroic) | In development |
| Unreal Engine | Yes | Yes (Steam/Proton, Lutris, Heroic) | Yes |

> Unreal Engine games are detected via GVAS .sav files. UE3 games and games using custom save formats are not supported.

---

## Changelog
See [CHANGELOG.md](CHANGELOG.md) for a complete history of all changes.

## Contributing

See [CONTRIBUTING.md](docs/CONTRIBUTING.md) for build instructions and contribution guidelines.

## License

GPLv3 see [LICENSE](LICENSE.md).

## Star History

<a href="https://www.star-history.com/?repos=marco007%2FSaveManager&type=date&legend=top-left">
 <picture>
   <source media="(prefers-color-scheme: dark)" srcset="https://api.star-history.com/image?repos=msh31/SaveManager&type=date&theme=dark&legend=top-left" />
   <source media="(prefers-color-scheme: light)" srcset="https://api.star-history.com/image?repos=msh31/SaveManager&type=date&legend=top-left" />
   <img alt="Star History Chart" src="https://api.star-history.com/image?repos=msh31/SaveManager&type=date&legend=top-left" />
 </picture>
</a>
