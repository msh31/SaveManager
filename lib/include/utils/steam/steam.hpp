#pragma once
#include <types.hpp>

namespace SteamHelper {
    std::vector<std::string> get_platform_steam_paths( );
    std::optional<fs::path>  get_steam_location( );
    std::vector<fs::path>    get_library_folders( );

    std::optional<std::string>   parse_steam_userid( );
    std::optional<SteamManifest> parse_app_manifest( const fs::path& acf_path );

    const std::unordered_map<uint32_t, SteamManifest>& get_app_manifests( );
} // namespace SteamHelper
