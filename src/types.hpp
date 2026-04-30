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

struct Game {
    PlatformType type;
    std::string appid;
    std::optional<std::string> game_id; 
    std::string game_name;
    std::filesystem::path save_path;
};
