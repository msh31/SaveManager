#pragma once
#include <utils/paths.hpp>

enum class PlatformType {
    UBISOFT = 1,
    ROCKSTAR,
    UNREAL,
    MINECRAFT,
    CUSTOM,
    GENERIC = 69,
    EA,
    PLAYSTATION,
    CDPROJEKTRED,
    PCGAMINGWIKI,
};

enum class LauncherType {
    OFFICIAL = 1,
    MODRINTH,
    CURSEFORGE,
    PRISM,
    MULTIMC,
};

struct Game {
        PlatformType type;          // display
        std::string platform_label; // detector-provided display name, see IDetector::name()
        std::string appid;
        std::optional<std::string> game_id; // ubi only
        std::string game_name;
        std::vector<fs::path> save_paths;
        LauncherType launcher;         // minecraft only | display
        bool show_parent_path = false; // display
};

enum class GameKeyKind { INVALID, STEAM_APPID, UBISOFT_ID, MINECRAFT, NAME, PATH };
struct GameKey {
        GameKeyKind kind;
        std::string value;
        // spaceship operator,
        auto operator<=>( const GameKey& ) const = default;
};
