#pragma once
namespace fs = std::filesystem;

namespace SteamHelper {
    std::vector<std::string> get_platform_steam_paths();
    std::optional<fs::path> get_steam_location();
    std::vector<fs::path> get_library_folders();
}
