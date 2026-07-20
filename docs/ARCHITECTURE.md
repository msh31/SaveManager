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

## Runtime Flow (GUI)
Order of events from launch to a rendered frame. Source: [`gui/src/main.cpp`](../gui/src/main.cpp).

1. `init_logger()`: installs the spdlog default logger with three sinks. An in-memory ringbuffer sink (last 500 lines, read by the Log tab), a daily file sink under the log dir, and a stdout sink (debug builds only).
2. `CWindowManager window;`: the constructor runs `setup_opengl()` (GLFW, OpenGL 3.3 core, GLAD) and `setup_imgui()` (context, fonts, backends). Nothing is drawn yet.
3. `CApp app;`: default-constructs all of `CApp`'s members (`CConfig`, `Blacklist`, `Translations`, `SteamManifestCache`, `UnrealNameCache`, the `CDetectionService` that borrows those four, `CUIManager`) but does no disk I/O yet, that happens in `app.init()`.
4. `window.restore_state(...)` / `window.show()`: applies the saved window geometry (read from `app.window_props()`, populated by the `CConfig` constructor) and shows the window before the heavier init work below, so the window appears promptly.
5. `app.init()`: applies the theme, then initializes `m_config`, `m_translations` (Ubisoft id/name lookup table, downloaded on first run if missing), `m_blacklist` (default seeds: *The Crew Motorfest*, *Skull and Bones*), `m_manifest_cache` (reads every locally-installed Steam game's manifest so Unreal detection can resolve a bare appid to a name), and `m_unreal_name_cache` (remembers those resolved names on disk at `<config>/cache/unreal_names.json` so a game keeps its name after being uninstalled) from disk in turn, each logs a warning and continues rather than aborting on failure. It then runs the one-off `Features::migrate_labels_to_tags()` label migration, registers the views (Dashboard, Editor, Transfer, About, Log, then Settings) into the UI manager, and constructs the background `CShader`.
6. `window.run( pre, ui )`: the frame loop. Each frame calls `pre()` (`app.render_shader()`: clears to the theme colour, draws the optional animated background), then wraps `ui()` (`app.render()`) in one fullscreen borderless ImGui window. Frame pacing uses two knobs: `glfwSwapInterval(1)` (vsync, set in `setup_opengl`) caps max FPS to the monitor refresh rate, and `glfwWaitEventsTimeout(1.0/60.0)` makes the loop event-driven, sleeping until input or ~16ms so an idle UI still wakes ~60x/s to animate the shader without busy-spinning. To uncap: set `glfwSwapInterval(0)` and lower the timeout or switch to `glfwPollEvents`. Changing the timeout alone will not beat the vsync ceiling.
7. Exit on `q` or window close, unwinding in reverse: `CApp` dtor (`m_task_runner.shutdown()`), `CConfig` dtor (saves `config.json`), `CWindowManager` dtor (ImGui and GLFW teardown).

Per frame, inside `app.render()`: `m_task_runner.update()` (fires `on_complete` for finished async tasks), `ThemeManager::apply_colors()` (re-applied every frame from config), `m_ui_manager.render()` (menubar, shell renders the active view, statusbar), `Notify::render_notifications()`, `ConfirmDialog::render()`.

> **Why re-apply the theme every frame instead of on change?** Deliberate. There is no config-change signal to hook into, and re-applying the colours is free in practice: it just copies values into the ImGui style struct, with no allocation or GPU work. Polling every frame is simpler than wiring up change detection and the cost does not show up. Do not turn this into an on-change system; there is nothing to win.

## Object Ownership
The object graph, top down:

- `main` (stack) owns: `CWindowManager`, `CApp`.
- `CApp` owns: `CConfig m_config`, `CUIManager m_ui_manager`, `Blacklist m_blacklist`, `Translations m_translations`, `SteamManifestCache m_manifest_cache`, `UnrealNameCache m_unreal_name_cache`, `CDetectionService m_detection` (borrows the four preceding members), `CTaskRunner m_task_runner`, `std::optional<CShader>`.
- `CUIManager` owns: `IShell` (the `CTabbarShell`), `vector<unique_ptr<CBaseView>>` (all views), `optional<CMenuBar>`, `optional<CStatusBar>`. Observes: `CBaseView* m_active_view` (raw, points into the vector it owns).
- `CDashboardView` owns: `CBackupsView`, its own `CTaskRunner`, the game cache (`m_game_cache`), the backup `std::future`. Borrows: `CConfig&`, `CDetectionService&`. `CTransferView` borrows the same `CConfig&`/`CDetectionService&`.

Detection is owned by `CApp`'s `CDetectionService m_detection`, constructed once and handed to every view that needs saves (`CDashboardView`, `CTransferView`) as a `CDetectionService&`. It is the single owner of both the in-flight scan and the last completed result, so there is exactly one scan running at a time and every view reads the same snapshot; see the Detection data flow below for how views consume it.

> **History:** before `CDetectionService` existed, `CDashboardView` and `CTransferView` each called `Detection::find_saves()` independently, so detection ran more than once per session and the two views could hold divergent snapshots. `CDetectionService` (introduced alongside the rest of this refactor) is the fix, it is now the single owner referenced by both views instead of each view running its own scan.

## Data Flows
The three operations worth tracing end to end. Following one relights most of the code it touches.

### Detection (Dashboard refresh / startup)
1. `CDashboardView::on_enter()` calls `m_detection.ensure_started()`, which kicks off a scan only if none has completed yet (`generation() == 0`) and none is already in flight; the toolbar's Refresh button calls `m_detection.refresh()` directly to force a rescan regardless. Either way, `CDetectionService::refresh()` runs `Detection::find_saves(blacklist, translations, manifest_cache, name_cache)` on a worker thread via `std::async`, stores the result under its own mutex, and bumps an atomic generation counter when done.
2. `find_saves` builds the platform detectors for the current OS, wraps each Lua plugin as a `CPlugin` detector, runs every detector's `find()` in parallel via `std::async`, and collects the `Game`s.
3. Post-processing, in order: dedup by `GameKey`, drop blacklisted games, drop games whose save paths are all missing or empty.
4. Every view's `render()` compares `m_detection.generation()` against its own last-seen generation; when it has advanced, the view copies `m_detection.snapshot()` into its local `m_games_snapshot`. On the Dashboard, a change in game count then calls `on_result_changed()`, which rebuilds the per-game cache (save files, backups, labels, conflict flags) on `m_task_runner` and swaps it in when done. `CTransferView` reads the same underlying result independently, through its own generation check.

**How a detector is chosen per platform:** on Windows, `find_saves` constructs Ubisoft/Rockstar/Unreal/EA/CD Projekt Red/Playstation Studios directly. Native Linux additionally constructs a standalone `CCDPRDetector` for The Witcher 2's native port before falling back to Wine for everything else. On Linux generally (and for non-native titles on macOS), the other detectors don't get constructed directly - instead `CWinePrefixDetector` walks every Wine/Proton prefix it's told about (one per Steam library, plus Heroic, plus Lutris) and, for each one, calls a small function each of those detectors registered for exactly this purpose. `CWinePrefixDetector` itself has no idea what Ubisoft or Rockstar even are; it just calls whatever it was handed. This is what lets a single detector's save-finding logic work both natively and under Wine without being duplicated, and lets a new detector opt into Wine support by registering its own function in `detection.cpp` instead of editing the Wine detector.

### Create backup (per-file Backup button)
1. `render_save_row`'s Backup button starts a `std::async` running `Features::backup_game(game, file, config)`.
2. Target is `backups/<sanitized game>/backup_<name>_<timestamp>.zip`, written to a `.tmp` sibling first.
3. `CZipArchive(MODE_CREATE_ARCHIVE)`: `add_to_archive(file)` adds the file or directory tree, `finalize_add()` builds `manifest.json` (per-file SHA-256 plus mtime) and writes it into the zip, then closes. The archive comment stores the original parent dir, used later as the restore destination.
4. On success the `.tmp` is atomically renamed to `.zip`. A failure removes the `.tmp` so a half-written archive never lands.
5. Variants: Mass Backup runs `backup_all_games` (re-walks the filesystem itself), "Backup All" for one game runs `backup_game_files` (uses the cached file list).

### Restore backup (Restore button)
1. `render_backup_row`'s Restore button calls `Features::restore_backup(zip, save_path, conflicts)` (synchronous, on the UI thread).
2. The restore destination comes from the archive comment, falling back to `save_path`.
3. If the destination already holds data, an `undo.zip` is written first (`backup_to_path`) so the restore is reversible.
4. `CZipArchive::extract_archive` per entry: resolve the output path safely (zip-slip guard via `weakly_canonical` plus a `..` check); if a target file exists and is newer than the archived copy, rename it aside to `<name>.savemgr-conflict-<mtime>` instead of overwriting; write the file; verify SHA-256 against the manifest (skip that file on mismatch) and restore the recorded mtime.
5. Any conflict files bubble back to the GUI as `m_pending_conflicts`, which opens the "Resolve conflict(s)" modal (Keep overwrites the live save, Delete discards the conflict copy). "Undo last restore" restores `undo.zip`.

## Backup On-Disk Layout
Backups live under `<config>/backups/<sanitized game name>/`. One game folder holds:

- `backup_<name>_<timestamp>.zip`: one backup. `<name>` defaults to the game name with spaces as underscores, `<timestamp>` is `YYYYMMDD_HHMMSS`. Written to a `.tmp` sibling first, then renamed in atomically (a failed write removes the `.tmp`, so a half-built archive never lands).
- `undo.zip`: written automatically before a restore would overwrite newer data, so "Undo last restore" can roll back. Removed once consumed.
- `tags.json`: map of backup filename to a list of user tags.

Inside each `.zip`:
- The save files, stored at paths relative to the save root.
- `manifest.json`: per-file `{ hash: SHA-256, mtime }`, used on restore to verify integrity and to restore timestamps. Optional, pre-manifest backups still restore.
- The zip archive *comment* holds the original save directory, used as the default restore destination.

Note: `.savemgr-conflict-<mtime>` markers do not live here. They are written into the game's real save directory when a restore would overwrite a newer file (see the Restore flow).

## Game Identity (`GameKey`)
Detection can find the same game more than once (for example a Steam entry and a plugin entry). `utils::get_game_identity_key()` collapses a `Game` to one `GameKey`, and `find_saves` merges entries that share a key by appending their save paths. Precedence, first match wins:

1. `STEAM_APPID`: `appid`, if set and not `"N/A"`.
2. `UBISOFT_ID`: `game_id`, if present.
3. `MINECRAFT`: `game_name`, for Minecraft entries (launchers merge by world name).
4. `NAME`: `game_name`, if non-empty.
5. `PATH`: the canonical first save path, as a last resort.
6. `INVALID`: nothing usable. Callers must check for this and skip the entry.

The same key also drives dashboard grouping: `m_game_cache` is keyed by `GameKey.value`.

---

## Library
- Config | handles all config logic such as creation, saving changes, populating a pre-set blacklist 

- Detection | handles the scanning logic to find savegames and also loads [plugins](SCRIPTING.md):
    - CUbisoftDetector | Searches for saves in known paths for Ubisoft games
    - CRockstarDetector | Searches for saves in known paths for Rockstar games
    - CUnrealDetector | Searches for saves matching Unreal's identity, determined by extension & header. When a save is found under a Wine/Proton prefix (identified only by a numeric Steam appid), the name is resolved from the local Steam library's own manifest for that appid and remembered on disk, so the name survives the game being uninstalled later
    - CWinePrefixDetector | Runs on Linux, and for non-native titles on macOS, in place of the platform-native detectors above. It has no built-in knowledge of Ubisoft/Rockstar/EA/Unreal/CDPR/Playstation Studios - each of those detectors instead registers a couple of small functions ("if you find a Wine/Proton prefix, call me with this folder") in `detection.cpp`, and `CWinePrefixDetector` just walks every known Wine prefix (Steam libraries, Heroic, Lutris) calling whatever was registered. This is also how a detector's actual save-finding logic ends up shared between its native path and its Wine path, instead of duplicated
    - CMinecraftDetector | Searches for minecraft worlds in known paths for various launchers 
    - CPlugin | Wraps a user Lua plugin as a detector (see [plugins](SCRIPTING.md))

- Features (Core features of the savemanager-lib)
    - Backup | handles backup and restoration of savefiles
    - Remote Transfer | allows users to transfer files over SFTP 
    - Save Editor | Editor for GTA San Andreas PC savefiles

- Network | a helper utility to download files and checking for updates

- Plugin | Finds, loads and runs user created plugins from ``path/to/config/plugins``

- Utils
    - Blacklist | A pre-made blacklist that is usre editable through the config
    - Steam | Helper functions for interacting with steam, including a cache of every locally-installed game's manifest (name, install dir) read straight from Steam's own library files - this is what backs Unreal's appid-to-name resolution
    - Translations | Handles Ubisoft gameid-to-name translations (downloaded on first run, refreshable from Settings)
    - UnrealNameCache | Remembers Unreal game names once resolved via the Steam manifest cache, in `<config>/cache/unreal_names.json`, so a game keeps its name after being uninstalled
    - ZipArchive | Creates, Extracts archives with helper methods for a manifest json with backup info

## GUI
Lifecycle and ownership for `CApp` and the views are covered under Runtime Flow and Object Ownership above. This section is the component index.

- App (`CApp`) | top-level object. GUI-specific note: notifications go through `Notify::show_notification()`, which is queued and mutex-guarded, safe to call from background threads, and never rendered directly from view logic.

- CDetectionService (`gui/src/backend/detection_service/`) | single owner of the detection scan and result for the whole GUI; see Object Ownership and the Detection data flow above.

- Views (each is a class with a `render()` method, lives under `gui/src/frontend/views/`)
    - DashboardTab | game cards, per-game backup / restore, mass backup
    - EditorTab | save editor (GTA San Andreas)
    - TransferTab | SFTP upload / download
    - LogTab | in-app log viewer, reads from the spdlog ringbuffer sink
    - SettingsTab | config editing
    - AboutTab | version and build info

## CLI (wip)
- Uses [CLI11](https://github.com/CLIUtils/CLI11) for argument parsing
- Log level is set to error by default (quiet for scripting use)
- Subcommands:
    - `list` | runs detection and prints all found games and their save files
    - `backup <game_id> [--all]` | creates a backup for a specific game or all games
    - `restore <game_id> <backup_id>` | restores a specific backup for a game

## Daemon (wip)
- Runs two detached threads, both started on launch and kept alive by a 50ms sleep loop:
    - CServer | exposes a socket for IPC with the GUI and CLI
    - CWatcher | watches paths from config for filesystem events (Linux: `inotify`, others: TBD)
        - Filters events through `extension_blocklist` and `g_extension_blocklist` before acting
        - Reacts to `IN_CREATE`, `IN_DELETE`, `IN_CLOSE_WRITE`
