#pragma once
namespace fs = std::filesystem;

namespace SteamHelper {
    std::vector<std::string> get_platform_steam_paths( );
    std::optional<fs::path>  get_steam_location( );
    std::vector<fs::path>    get_library_folders( );

    std::unordered_map<uint32_t, std::string> parse_app_manifest( const fs::path& acf_path );

} // namespace SteamHelper
