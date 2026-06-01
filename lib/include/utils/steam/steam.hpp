#pragma once
#include <types.hpp>

namespace SteamHelper {
    std::vector<std::string> get_platform_steam_paths( );
    std::optional<fs::path>  get_steam_location( );
    std::vector<fs::path>    get_library_folders( );

    std::optional<SteamManifest> parse_app_manifest( const fs::path& acf_path );

} // namespace SteamHelper
