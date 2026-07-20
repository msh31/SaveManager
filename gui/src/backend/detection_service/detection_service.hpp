#pragma once
#include <detection/detection.hpp>

// single owner of the detection result
class CDetectionService {
    public:
        CDetectionService(
            const Blacklist& blacklist, const Translations& translations, const SteamManifestCache& manifest_cache,
            UnrealNameCache& name_cache )
            : m_blacklist( blacklist ), m_translations( translations ), m_manifest_cache( manifest_cache ),
              m_name_cache( name_cache ) {}

        void refresh( );
        void ensure_started( );

        bool is_refreshing( ) const;

        // bumped once per completed scan; compare against a locally-cached value to notice new results
        uint64_t generation( ) const { return m_generation.load( ); }

        std::vector<Game> snapshot( ) const;

        double last_duration( ) const { return m_last_duration.load( ); }

    private:
        const Blacklist& m_blacklist;
        const Translations& m_translations;
        const SteamManifestCache& m_manifest_cache;
        UnrealNameCache& m_name_cache;

        mutable std::mutex m_mutex;
        std::vector<Game> m_result;
        std::atomic<uint64_t> m_generation{ 0 };
        std::atomic<double> m_last_duration{ 0.0 };
        std::future<void> m_future;
};
