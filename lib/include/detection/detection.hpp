#pragma once
#include <types.hpp>

#include <utils/blacklist/blacklist.hpp>
#include <utils/steam/steam.hpp>
#include <utils/translations/translations.hpp>
#include <utils/unreal_name_cache/unreal_name_cache.hpp>

namespace Detection {
    std::vector<Game> find_saves(
        const Blacklist&, const Translations&, const SteamManifestCache&, UnrealNameCache& );
}; // namespace Detection
