#pragma once
#include <detection/game.hpp>

#include <utils/steam/steam.hpp>
#include <utils/translations/translations.hpp>
#include <utils/unreal_name_cache/unreal_name_cache.hpp>

struct PcgwEntry {
        std::string os;
        std::string raw_path;
};

struct DetectorContext {
        const Translations& translations;
        const SteamManifestCache& manifest_cache;
        UnrealNameCache& name_cache;
        const std::unordered_map<uint32_t, std::vector<PcgwEntry>>& pcgw_entries;
};

using WineScanHook = std::vector<Game> ( * )( const fs::path&, const DetectorContext& );
