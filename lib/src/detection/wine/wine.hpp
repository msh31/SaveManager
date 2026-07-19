#pragma once
#include "../idetector.hpp"
#include <utils/steam/steam.hpp>
#include <utils/translations/translations.hpp>
#include <utils/unreal_name_cache/unreal_name_cache.hpp>

class CWinePrefixDetector : public IDetector {
    public:
        CWinePrefixDetector(
            fs::path prefix, const Translations& translations, const SteamManifestCache& manifest_cache,
            UnrealNameCache& name_cache )
            : m_path( std::move( prefix ) ), m_translations( translations ), m_manifest_cache( manifest_cache ),
              m_name_cache( name_cache ) {};

        std::expected<std::vector<Game>, SMError> find( ) override;

        std::string_view name( ) const override;

    private:
        fs::path m_path = { }; // must be set
        const Translations& m_translations;
        const SteamManifestCache& m_manifest_cache;
        UnrealNameCache& m_name_cache;
};
