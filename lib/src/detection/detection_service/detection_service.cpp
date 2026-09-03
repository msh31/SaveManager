#include "../pcgw/pcgw.hpp"
#include <detection/detection_service.hpp>
#include <logger.hpp>

CDetectionService& CDetectionService::get( ) {
    static CDetectionService instance;
    return instance;
}

void CDetectionService::ensure_started( ) {
    if ( m_generation.load( ) == 0 && !is_refreshing( ) ) refresh( );
}

bool CDetectionService::is_refreshing( ) const {
    return m_future.valid( ) && m_future.wait_for( std::chrono::seconds( 0 ) ) != std::future_status::ready;
}

std::vector<Game> CDetectionService::snapshot( ) const {
    std::lock_guard lock( m_mutex );
    return m_result;
}

void CDetectionService::init( ) {
    if ( !m_translations.init( ) ) {
        SPDLOG_WARN( "Failed to initialize translations! Expect missing games!" );
    }
    if ( !m_manifest_cache.init( ) ) {
        SPDLOG_WARN( "Failed to initialize Steam manifest cache! Expect missing Unreal game names!" );
    }
    if ( !m_name_cache.init( ) ) {
        SPDLOG_WARN( "Failed to load cached Unreal game names!" );
    }
    if ( !m_blacklist.init( ) ) {
        SPDLOG_WARN( "Failed to initialize blacklist!" );
    }

    m_pcgw_entries = CPCGamingWikiDetector::load_manifest( );
    if ( m_pcgw_entries.empty( ) ) {
        SPDLOG_WARN( "Failed to load the manifest, expect missing games!" );
    }
    m_detectors = Detection::build_detectors( m_translations, m_manifest_cache, m_name_cache, m_pcgw_entries );
}


void CDetectionService::refresh( ) {
    if ( is_refreshing( ) ) return;

    auto start = std::chrono::steady_clock::now( );
    m_future = std::async( std::launch::async, [this, start] {
        std::vector<std::pair<IDetector*, std::future<std::expected<std::vector<Game>, SMError>>>> futures = { };
        std::vector<Game> games = { };

        // collect detectors and launch them
        for ( const auto& detector : m_detectors ) {
            futures.emplace_back(
                detector.get( ),
                std::async( std::launch::async, [d = detector.get( )]( ) -> std::expected<std::vector<Game>, SMError> {
                    return d->find( );
                } ) );
        }

        while ( !futures.empty( ) ) {
            std::erase_if( futures, [&]( auto& pair ) {
                auto& [detector, future] = pair;
                bool ready = false;

                // COMMENT: would bite in the future but is fine since I don't plan on adding more detectors
                if ( future.valid( ) &&
                     future.wait_for( std::chrono::milliseconds( 100 ) ) == std::future_status::ready ) {
                    ready = true;
                }
                if ( ready ) {
                    try {
                        auto res = future.get( );
                        if ( res.has_value( ) ) {
                            std::ranges::move( res.value( ), std::back_inserter( games ) );

                            try {
                                // INTERNAL > PCGW Manifest
                                std::erase_if( games, [&]( const Game& game ) {
                                    if ( game.type != PlatformType::PCGAMINGWIKI ) return false;

                                    return std::ranges::any_of( games, [&]( const Game& other ) {
                                        return other.type != PlatformType::PCGAMINGWIKI &&
                                               other.game_name == game.game_name;
                                    } );
                                } );

                                // DE-DUPLICATION
                                games = std::move( Detection::de_duplicate( games ) );

                                games = std::move( Detection::merge_by_path( games ) );

                                // BLACKLIST
                                std::erase_if( games, [this]( const Game& game ) {
                                    bool blacklisted = m_blacklist.is_blacklisted( game.game_name );
                                    if ( blacklisted )
                                        SPDLOG_INFO( "[Detection] {} is blacklisted, removing.", game.game_name );
                                    return blacklisted;
                                } );

                                // VALID PATH CHECK
                                std::erase_if( games, []( const Game& game ) {
                                    bool has_valid_path =
                                        std::ranges::any_of( game.save_paths, []( const fs::path& p ) {
                                            return fs::is_directory( p ) && !fs::is_empty( p );
                                        } );
                                    if ( !has_valid_path )
                                        SPDLOG_INFO(
                                            "[Detection] {} has no valid save paths ({} checked), removing.",
                                            game.game_name, game.save_paths.size( ) );
                                    return !has_valid_path;
                                } );

                                {
                                    std::lock_guard lock( m_mutex );
                                    m_result = games; // std::move( games );
                                }

                                m_last_duration.store(
                                    std::chrono::duration<double>( std::chrono::steady_clock::now( ) - start )
                                        .count( ) );
                                m_generation.fetch_add( 1 );

                            } catch ( fs::filesystem_error& ex ) {
                                SPDLOG_ERROR( "filter chain failed: {}", ex.what( ) );
                            }

                        } else {
                            SPDLOG_WARN( "{} detection failed", detector->name( ) );
                        }
                    } catch ( fs::filesystem_error& ex ) {
                        SPDLOG_ERROR(
                            "{} failed in {} because: {}", detector->name( ), ex.path1( ).string( ),
                            ex.code( ).message( ) );
                    }
                    return true;
                }
                return false;
            } );
            m_pending_count = futures.size( );
        }

        if ( games.empty( ) ) {
            SPDLOG_ERROR( "No savegames found!" );
        }
    } );
}