#include "detection/detection.hpp"
#include "plugin/plugin.hpp"

#include "utils/blacklist/blacklist.hpp"
#include "utils/paths.hpp"
#include <utils/utils.hpp>

#include <utils/steam/steam.hpp>

#include "detection/minecraft/minecraft.hpp"

#if defined __APPLE__ || defined __linux__
    #include <detection/wine/wine.hpp>
#endif

#include <detection/rsg/rsg.hpp>
#include <detection/ubi/ubi.hpp>
#include <detection/unreal/unreal.hpp>

std::vector<Game> Detection::find_saves( ) {
    std::vector<std::unique_ptr<IDetector>> detectors;
    std::vector<std::future<std::expected<std::vector<Game>, SMError>>> detection_futures;

    std::vector<Game> games;

#ifdef _WIN32
    detectors.emplace_back( std::make_unique<CUbisoftDetector>( ) );
    detectors.emplace_back( std::make_unique<CRockstarDetector>( ) );
    detectors.emplace_back( std::make_unique<CUnrealDetector>( ) );
#endif

#ifdef __linux__
    auto prefixes = SteamHelper::get_library_folders( );

    // steam
    for ( const auto& prefix : prefixes ) {
        detectors.emplace_back( std::make_unique<CWinePrefixDetector>( prefix / "steamapps/compatdata" ) );
    }

    // TODO: improve resolved paths for heroic and lutris
    // heroic
    if ( fs::exists( paths::heroic_dir( ) ) ) {
        detectors.emplace_back( std::make_unique<CWinePrefixDetector>( paths::heroic_dir( ) / "Prefixes/default" ) );
    }

    // lutris
    if ( fs::exists( paths::lutris_dir( ) ) ) {
        detectors.emplace_back( std::make_unique<CWinePrefixDetector>( paths::lutris_dir( ) ) );
    }
#endif

#ifdef __APPLE__
    // native
    detectors.emplace_back( std::make_unique<CUnrealDetector>( ) );

    // non-native
    auto prefixes = SteamHelper::get_library_folders( );

    for ( const auto& prefix : prefixes ) {
        detectors.emplace_back( std::make_unique<CWinePrefixDetector>( prefix ) );
    }
#endif
    detectors.emplace_back( std::make_unique<CMinecraftDetector>( ) );

    for ( const auto& detector : detectors ) {
        detection_futures.emplace_back(
            std::async( std::launch::async, [d = detector.get( )]( ) -> std::expected<std::vector<Game>, SMError> {
                return d->find( );
            } ) );
    }

    // c++23 ftw; wait for async completions and insert them
    for ( auto&& [detector, future] : std::views::zip( detectors, detection_futures ) ) {
        if ( future.valid( ) ) {
            auto res = future.get( );
            if ( res.has_value( ) ) std::ranges::move( res.value( ), std::back_inserter( games ) );
            else
                SPDLOG_WARN( "{} detection failed", detector->name( ) );
        }
    }

    // cool lua support
    int plugin_count = 0;
    for ( const auto& plugin : fs::recursive_directory_iterator(
              paths::plugin_dir( ),
              fs::directory_options::skip_permission_denied | fs::directory_options::follow_directory_symlink ) ) {
        if ( plugin.path( ).extension( ) != ".lua" ) continue;
        if ( !fs::is_regular_file( plugin ) ) continue;
        plugin_count++;

        CPlugin plugins( plugin );
    }
    if ( plugin_count > 0 ) SPDLOG_INFO( "Loaded {} plugins!", plugin_count );

    if ( games.empty( ) ) {
        SPDLOG_ERROR( "No savegames found!" );
    }

    std::map<GameKey, size_t> seen;
    std::vector<Game> deduped;
    for ( size_t i = 0; i < games.size( ); i++ ) {
        auto& game = games[i];
        auto key = utils::get_game_identity_key( game );
        if ( key.kind == GameKeyKind::INVALID ) continue;

        if ( seen.contains( key ) ) {
            deduped[seen[key]].save_paths.insert(
                deduped[seen[key]].save_paths.end( ), game.save_paths.begin( ), game.save_paths.end( ) );
        } else {
            deduped.push_back( game );
            seen[key] = deduped.size( ) - 1;
        }
    }
    games = std::move( deduped );

    std::erase_if( games, []( const Game& game ) { return Blacklist::is_blacklisted( game.game_name ); } );
    std::erase_if( games, []( const Game& game ) {
        return std::ranges::none_of(
            game.save_paths, []( const fs::path& p ) { return fs::is_directory( p ) && !fs::is_empty( p ); } );
    } );

    return games;
}
