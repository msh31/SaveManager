#pragma once
#include "../../utils/save_helper/save_helper.hpp"
#include "../detector_context.hpp"
#include "../idetector.hpp"
#include <utils/steam/steam.hpp>

// Where Wine/Proton roots resolve to, so resolve() can be shared between the native
// and Wine paths - only the "fixed OS root" tokens differ between the two.
struct WineRootCtx {
        fs::path user_home; // drive_c/users/<name>
        fs::path drive_c;
};

class CPCGamingWikiDetector : public IDetector {
    public:
        static constexpr std::string_view PLATFORM_LABEL = "PCGamingWiki";

        CPCGamingWikiDetector(
            const SteamManifestCache& manifest_cache,
            const std::unordered_map<uint32_t, std::vector<PcgwEntry>>& entries );
        static std::unordered_map<uint32_t, std::vector<PcgwEntry>> load_manifest( );

        std::expected<std::vector<Game>, SMError> find( ) override;

        std::string_view name( ) const override;

        static std::vector<Game> scan_wine_user( const fs::path& user_home, const DetectorContext& ctx );

    private:
        static std::optional<fs::path>
        resolve( const std::string& raw_path, const SteamManifest* manifest, const WineRootCtx* wine );

        const SteamManifestCache& m_manifest_cache;
        std::unordered_map<uint32_t, std::vector<PcgwEntry>> m_entries;
};
