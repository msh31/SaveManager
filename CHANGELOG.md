# 📓 SaveManager Changelog
this file holds the complete changelog since the first commit, the changelog is formatted as follows:

```
COMMIT_ID | COMMIT_MSG | DATE 
- Major changes
- Fixes
- Removals
- Other
```

---

## ``9847ffa`` | Initial commit | Aug 31 15:29:47 2025
- first commit, includes CLI version, this is the GUI version.

## ``6742c77`` | Initial GUI commit | Aug 31 15:35:17 2025 
- Uploaded blank Avalonia .NET app
- Removed CLI version
- Updated README to reflect GUI version

## ``fe296d9`` | Update README.md | Aug 31 16:54:42 2025
- Added reference to the CLI version

## ``fd27a27`` | feat (wip): front-end design mostly complete | Aug 31 20:38:06 2025
- Implemented GUI created by Claude from a sketch made by me and heavy refinement

## ``6c7edc0`` | Merge #1 | Aug 31 20:38:17 2025
- merged changes ``fe296d9`` & ``fd27a27``

## ``3e47937`` | feat: Implemented smart UserControl switching & add Settings view | Aug 31 22:03:39 2025
- Move views to seperate UI/ directory
- Create Settings & Dashboard views & Sidebar component

## ``5c337db`` | feat: add JSON config manager | Aug 31 22:35:21 2025
- Created Modules/ directory
- Filled Modules/ directory with 3 main providers
- Create ConfigManager (JSON format)

## ``9fc8873`` | feat: add SentiLog logging, move UI files around, add: cross-platform ubi folder detection | Sep 1 03:15:15 2025 
- Added ubisoftfolder path to config with detection logic (Added to toggle in Settings panel)
- Disabled UE/RSG toggles in settings
- Moved Sidebar to UI/Components/
- Moved views to UI/Views
- MainWindow remains in UI/
- Create blank Sync/Backup Manager classes
- Create Globals class 
> _Note: user must select the right account in the ubi selection themselves_