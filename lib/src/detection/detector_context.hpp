#pragma once
#include <types.hpp>

#include <utils/steam/steam.hpp>
#include <utils/translations/translations.hpp>
#include <utils/unreal_name_cache/unreal_name_cache.hpp>

struct DetectorContext {
        const Translations& translations;
        const SteamManifestCache& manifest_cache;
        UnrealNameCache& name_cache;
};

using WineScanHook = std::vector<Game> ( * )( const fs::path&, const DetectorContext& );
