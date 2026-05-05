# Creating Lua Scripts for SaveManager
Lua scripting allows you to add custom game detection for games not natively supported by SaveManager. This feature is **experimental**! 

## Quick Start
1. Create a `.lua` file in your plugins directory:
   - **Linux**: `~/.config/savemanager/plugins/`
   - **Windows**: `C:\Users\<username>\savemanager\plugins\`
   - **macOS**: `~/Library/Application Support/savemanager/plugins/`

2. Define a `find_saves` function that returns a table of game entries:
```lua
function find_saves()
    local games = {}

    games[1] = {
        game_name = "My Game",
        appid = "123456",
        save_path = "/path/to/saves"
    }

    return games
end
```

3. Restart SaveManager and your custom games will appear automatically.

## Available API Functions
| Function | Signature | Description |
|----------|-----------|-------------|
| `path_exists` | `path_exists(path: string) -> boolean` | Check if a file or directory exists |
| `home_dir` | `home_dir() -> string` | Get the user's home directory |
| `list_dir` | `list_dir(path: string) -> table` | List all files/directories in a path |

All standard Lua base library functions (`print`, `type`, `tostring`, etc.) are also available.

## Game Entry Fields
Each entry in the returned table must contain:

| Field | Type | Description |
|-------|------|-------------|
| `game_name` | string | Display name of the game |
| `appid` | string | Steam AppID for artwork, or `"N/A"` if not on Steam |
| `save_path` | string | Path to the save directory |

## Example Scripts
### Basic Example

```lua
function find_saves()
    local games = {}
    local home = home_dir()

    local save_path = home .. "/.local/share/MyGame/saves"

    if path_exists(save_path) then
        games[1] = {
            game_name = "My Game",
            appid = "123456",
            save_path = save_path
        }
    end

    return games
end
```

### Multiple Games with Directory Scanning
```lua
function find_saves()
    local games = {}
    local index = 1
    local home = home_dir()
    local games_root = home .. "/Games"

    if not path_exists(games_root) then
        return games
    end

    local dirs = list_dir(games_root)

    for i, path in ipairs(dirs) do
        local save_path = path .. "/saves"

        if path_exists(save_path) then
            games[index] = {
                game_name = path:match("^.+/(.+)$") or path,
                appid = "N/A",
                save_path = save_path
            }
            index = index + 1
        end
    end

    return games
end
```

## Configuration

Scripts can optionally define a `config` table to control plugin behavior:

```lua
config = {
    show_parent_path = true
}
```

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `show_parent_path` | boolean | `false` | Show the parent directory path in the UI |

## Notes
- Scripts run with the same permissions as SaveManager
- The `find_saves` function must return a table even if no games are found

## Troubleshooting
- **Script not loading**: Verify the `.lua` file is in the correct `plugins/` directory and not a subdirectory
- **No games detected**: Check that `save_path` exists using `path_exists()`
- **Errors**: Ensure `find_saves` returns a properly structured table with all required fields
