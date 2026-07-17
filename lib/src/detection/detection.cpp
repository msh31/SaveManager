// clang-format off
#include <detection/detection.hpp>
#include "../plugin/plugin.hpp"

#include "utils/paths.hpp"
#include <utils/utils.hpp>

#include <utils/steam/steam.hpp>

#include "minecraft/minecraft.hpp"
#if defined __APPLE__ 
    #include "wine/wine.hpp"
#include "unreal/unreal.hpp"
#elif defined  __linux__
    #include "wine/wine.hpp"
#else //assuming windows
#include "rsg/rsg.hpp"
#include "ubi/ubi.hpp"
#include "unreal/unreal.hpp"
#include "ea/ea.hpp"
#endif
// clang-format on

std::vector<Game> Detection::find_saves( const Blacklist& blacklist, const Translations& translations ) {
    std::vector<std::unique_ptr<IDetector>> detectors;
    std::vector<std::future<std::expected<std::vector<Game>, SMError>>> detection_futures;

    std::vector<Game> games = { };

#ifdef _WIN32
    detectors.emplace_back( std::make_unique<CUbisoftDetector>( translations ) );
    detectors.emplace_back( std::make_unique<CRockstarDetector>( translations ) );
    detectors.emplace_back( std::make_unique<CUnrealDetector>( translations ) ); // UE4/5 only
    detectors.emplace_back( std::make_unique<CElectronicArtsDetector>( translations ) );
#endif

#ifdef __linux__
    auto prefixes = SteamHelper::get_library_folders( );

    // steam
    for ( const auto& prefix : prefixes ) {
        detectors.emplace_back(
            std::make_unique<CWinePrefixDetector>( prefix / "steamapps/compatdata", translations ) );
    }

    // TODO: improve resolved paths for heroic and lutris
    // heroic
    if ( fs::exists( paths::heroic_dir( ) ) ) {
        detectors.emplace_back(
            std::make_unique<CWinePrefixDetector>( paths::heroic_dir( ) / "Prefixes/default", translations ) );
    }

    // lutris
    if ( fs::exists( paths::lutris_dir( ) ) ) {
        detectors.emplace_back( std::make_unique<CWinePrefixDetector>( paths::lutris_dir( ), translations ) );
    }
#endif

#ifdef __APPLE__
    // native
    detectors.emplace_back( std::make_unique<CUnrealDetector>( translations ) );

    // non-native
    auto prefixes = SteamHelper::get_library_folders( );

    for ( const auto& prefix : prefixes ) {
        detectors.emplace_back( std::make_unique<CWinePrefixDetector>( prefix, translations ) );
    }
#endif

    detectors.emplace_back( std::make_unique<CMinecraftDetector>( ) );

    // cool lua support
    int plugin_count = 0;
    for ( const auto& plugin : fs::recursive_directory_iterator(
              paths::plugin_dir( ),
              fs::directory_options::skip_permission_denied | fs::directory_options::follow_directory_symlink ) ) {
        if ( plugin.path( ).extension( ) != ".lua" ) continue;
        if ( !fs::is_regular_file( plugin ) ) continue;
        plugin_count++;

        detectors.emplace_back( std::make_unique<CPlugin>( plugin ) );
    }
    if ( plugin_count > 0 ) SPDLOG_INFO( "Loaded {} plugins!", plugin_count );

    for ( const auto& detector : detectors ) {
        detection_futures.emplace_back(
            std::async( std::launch::async, [d = detector.get( )]( ) -> std::expected<std::vector<Game>, SMError> {
                return d->find( );
            } ) );
    }

    // c++23 ftw; wait for async completions and insert them
    for ( auto&& [detector, future] : std::views::zip( detectors, detection_futures ) ) {
        if ( future.valid( ) ) {
            try {
                auto res = future.get( );
                if ( res.has_value( ) ) std::ranges::move( res.value( ), std::back_inserter( games ) );
                else
                    SPDLOG_WARN( "{} detection failed", detector->name( ) );
            } catch ( fs::filesystem_error& ex ) {
                SPDLOG_ERROR(
                    "{} failed in {} because: {}", detector->name( ), ex.path1( ).string( ), ex.code( ).message( ) );
            }
        }
    }

    if ( games.empty( ) ) {
        SPDLOG_ERROR( "No savegames found!" );
    }

    std::map<GameKey, size_t> seen{ };
    std::vector<Game> deduped = { };
    size_t game_count = games.size( );
    for ( size_t i = 0; i < game_count; i++ ) {
        auto& game = games[i];
        auto key = utils::get_game_identity_key( game );
        if ( key.kind == GameKeyKind::INVALID ) continue;

        if ( seen.contains( key ) ) {
            SPDLOG_INFO( "[Detection] {} has been seen already! removing duplicate.", key.value );
            deduped[seen[key]].save_paths.insert(
                deduped[seen[key]].save_paths.end( ), game.save_paths.begin( ), game.save_paths.end( ) );
        } else {
            deduped.push_back( game );
            seen[key] = deduped.size( ) - 1;
        }
    }
    games = std::move( deduped );

    std::erase_if( games, [&blacklist]( const Game& game ) {
        bool blacklisted = blacklist.is_blacklisted( game.game_name );
        if ( blacklisted ) SPDLOG_INFO( "[Detection] {} is blacklisted, removing.", game.game_name );
        return blacklisted;
    } );

    std::erase_if( games, []( const Game& game ) {
        bool has_valid_path = std::ranges::any_of(
            game.save_paths, []( const fs::path& p ) { return fs::is_directory( p ) && !fs::is_empty( p ); } );
        if ( !has_valid_path )
            SPDLOG_INFO(
                "[Detection] {} has no valid save paths ({} checked), removing.", game.game_name,
                game.save_paths.size( ) );
        return !has_valid_path;
    } );

    return games;
}
