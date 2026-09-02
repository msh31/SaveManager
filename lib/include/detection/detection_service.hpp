#pragma once
#include <detection/detection.hpp>

// single owner of the detection result
class CDetectionService {
    public:
        CDetectionService( const Blacklist& blacklist );
        ~CDetectionService( );

        void init( );
        void refresh( );
        void ensure_started( );

        bool is_refreshing( ) const;

        // bumped once per completed scan; compare against a locally-cached value to notice new results
        uint64_t generation( ) const { return m_generation.load( ); }

        std::vector<Game> snapshot( ) const;

        double last_duration( ) const { return m_last_duration.load( ); }
        std::pair<uint64_t, uint64_t> get_detection_progress( ) const {
            std::pair<uint64_t, uint64_t> prog = { };

            prog.first = m_detectors.size( ) - m_pending_count; // current
            prog.second = m_detectors.size( );                  // total

            return prog;
        }

    private:
        const Blacklist& m_blacklist;
        Translations m_translations;
        SteamManifestCache m_manifest_cache;
        UnrealNameCache m_name_cache;
        std::unordered_map<uint32_t, std::vector<PcgwEntry>> m_pcgw_entries = { };
        std::vector<std::unique_ptr<IDetector>> m_detectors;

        mutable std::mutex m_mutex;
        std::vector<Game> m_result;
        std::atomic<uint64_t> m_generation{ 0 };
        std::atomic<double> m_last_duration{ 0.0 };
        std::future<void> m_future;

        std::atomic<std::size_t> m_pending_count = 0;
};
