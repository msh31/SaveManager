#pragma once

struct RemoteEntry {
    std::string name;
    bool is_directory;
};

enum class DetectionError {
    PathNotFound,
    PermissionDenied,
    NoSavesFound,
};

enum class PlatformType {
    UBISOFT = 1,
    ROCKSTAR,
    UNREAL,
    PSP,
    PPSSPP,
    MINECRAFT,
    CUSTOM
};

enum class LauncherType {
    OFFICIAL = 1,
    MODRINTH,
    CURSEFORGE,
    PRISM,
    MULTIMC,
};

struct Game {
    PlatformType type; //display
    std::string appid;
    std::optional<std::string> game_id; 
    std::string game_name;
    std::filesystem::path save_path;
    LauncherType launcher; //minecraft only | display
    bool show_parent_path; //display
};
