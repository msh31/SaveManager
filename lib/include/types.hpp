#pragma once

struct RemoteEntry {
        std::string name;
        bool        is_directory;
};

enum class DetectionError {
    PathNotFound,
    PermissionDenied,
    NoSavesFound,
};

enum class PlatformType { UBISOFT = 1, ROCKSTAR, UNREAL, PSP, PPSSPP, MINECRAFT, CUSTOM, GENERIC = 69 };

enum class LauncherType {
    OFFICIAL = 1,
    MODRINTH,
    CURSEFORGE,
    PRISM,
    MULTIMC,
};

struct Game {
        PlatformType               type; // display
        std::string                appid;
        std::optional<std::string> game_id; // ubi only
        std::string                game_name;
        // fs::path                   save_path;
        std::vector<fs::path> save_paths;
        LauncherType          launcher;                 // minecraft only | display
        bool                  show_parent_path = false; // display
};

struct ScheduleEntry {
        bool    enabled;
        int     interval_hours;
        int64_t last_backup_time;

        PlatformType type = PlatformType::GENERIC;
        std::string  appid;
        std::string  game_name;

        fs::path              save_path;
        std::vector<fs::path> included_saves;
};

struct BackupEntry {
        fs::path              name;
        fs::path              save_path;
        std::vector<fs::path> entries;
        size_t                size;
};

struct ManifestSavePath {
        bool is_mac;
        bool is_windows;
        bool is_linux;
        bool fully_resolved;

        fs::path unresolved_path;
        fs::path resolved_path;

        std::vector<std::string> tags;
};

struct SteamManifest {
        uint32_t    appid;
        std::string name;
        std::string install_dir;
        fs::path    library_dir;
};

enum class GameKeyKind { INVALID, STEAM_APPID, UBISOFT_ID, MINECRAFT, NAME, PATH };
struct GameKey {
        GameKeyKind kind;
        std::string value;
        // spaceship operator,
        auto operator<=>( const GameKey& ) const = default;
};
