#pragma once
#include "../idetector.hpp"
#include <utils/steam/steam.hpp>
#include <utils/unreal_name_cache/unreal_name_cache.hpp>

class CUnrealDetector : public IDetector {
    public:
        static constexpr std::string_view PLATFORM_LABEL = "Unreal";

        CUnrealDetector( const SteamManifestCache& manifest_cache, UnrealNameCache& name_cache )
            : m_manifest_cache( manifest_cache ), m_name_cache( name_cache ) {}
        std::expected<std::vector<Game>, SMError> find( ) override;

        std::string_view name( ) const override;

    private:
        friend class CWinePrefixDetector;

        static std::vector<Game> scan(
            fs::path, const SteamManifestCache& manifest_cache, UnrealNameCache& name_cache );
        const SteamManifestCache& m_manifest_cache;
        UnrealNameCache& m_name_cache;

        // uses the gvas header to identify saves
        static std::vector<Game> scan_recursive(
            const fs::path& path, const SteamManifestCache& manifest_cache, UnrealNameCache& name_cache );
        static std::optional<fs::path> resolve_save_games( const fs::path& folder ); // windows only
};
