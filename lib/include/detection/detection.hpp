#pragma once
#include <detection/game.hpp>

#include <utils/blacklist/blacklist.hpp>
#include <utils/steam/steam.hpp>
#include <utils/translations/translations.hpp>
#include <utils/unreal_name_cache/unreal_name_cache.hpp>

class IDetector;

namespace Detection {
    std::vector<Game> find_saves( const Blacklist&, const Translations&, const SteamManifestCache&, UnrealNameCache& );

    std::vector<std::unique_ptr<IDetector>> build_detectors(
        const Translations&, const SteamManifestCache&, UnrealNameCache&,
        const std::unordered_map<uint32_t, std::vector<PcgwEntry>>& );

    std::vector<Game> de_duplicate( const std::vector<Game>& );
}; // namespace Detection
