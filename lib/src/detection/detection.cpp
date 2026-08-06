#include "../plugin/plugin.hpp"
#include "detector_context.hpp"
#include <detection/detection.hpp>

#include "utils/paths.hpp"
#include <utils/utils.hpp>

#include <utils/steam/steam.hpp>

#include "minecraft/minecraft.hpp"
#include "pcgw/pcgw.hpp"
#include "rsg/rsg.hpp"
#include "ubi/ubi.hpp"
#include "unreal/unreal.hpp"

#if defined( __linux__ ) || defined( __APPLE__ )
    #include "wine/wine.hpp"
#endif

std::vector<std::unique_ptr<IDetector>> Detection::build_detectors(
    const Translations& translations, const SteamManifestCache& manifest_cache, UnrealNameCache& name_cache,
    const std::unordered_map<uint32_t, std::vector<PcgwEntry>>& pcgw_entries ) {

    std::vector<std::unique_ptr<IDetector>> detectors = { };
    DetectorContext ctx{ translations, manifest_cache, name_cache, pcgw_entries };

#ifdef _WIN32
    detectors.emplace_back( std::make_unique<CUbisoftDetector>( translations ) );
    detectors.emplace_back( std::make_unique<CRockstarDetector>( translations ) );
    detectors.emplace_back( std::make_unique<CUnrealDetector>( manifest_cache, name_cache ) ); // UE4/5 only
#endif

#if defined( __linux__ ) || defined( __APPLE__ )
    std::vector<WineScanHook> wine_prefix_hooks = { CUbisoftDetector::scan_wine_prefix };
    std::vector<WineScanHook> wine_user_hooks = {
        CUbisoftDetector::scan_wine_user, CRockstarDetector::scan_wine_user, CUnrealDetector::scan_wine_user,
        CPCGamingWikiDetector::scan_wine_user };
#endif

#ifdef __linux__
    auto prefixes = SteamHelper::get_library_folders( );
    // steam
    for ( const auto& prefix : prefixes ) {
        detectors.emplace_back(
            std::make_unique<CWinePrefixDetector>(
                prefix / "steamapps/compatdata", ctx, wine_prefix_hooks, wine_user_hooks ) );
    }

    // TODO: improve resolved paths for heroic and lutris
    // heroic
    if ( fs::exists( paths::heroic_dir( ) ) ) {
        detectors.emplace_back(
            std::make_unique<CWinePrefixDetector>(
                paths::heroic_dir( ) / "Prefixes/default", ctx, wine_prefix_hooks, wine_user_hooks ) );
    }

    // lutris
    if ( fs::exists( paths::lutris_dir( ) ) ) {
        detectors.emplace_back(
            std::make_unique<CWinePrefixDetector>( paths::lutris_dir( ), ctx, wine_prefix_hooks, wine_user_hooks ) );
    }
#endif

#ifdef __APPLE__
    // native
    detectors.emplace_back( std::make_unique<CUnrealDetector>( manifest_cache, name_cache ) );

    // non-native
    auto prefixes = SteamHelper::get_library_folders( );

    for ( const auto& prefix : prefixes ) {
        detectors.emplace_back(
            std::make_unique<CWinePrefixDetector>( prefix, ctx, wine_prefix_hooks, wine_user_hooks ) );
    }
#endif

    detectors.emplace_back( std::make_unique<CPCGamingWikiDetector>( manifest_cache, pcgw_entries ) );
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

    return detectors;
}

std::vector<Game> Detection::de_duplicate( const std::vector<Game>& games ) {
    if ( games.empty( ) ) return games;

    std::map<GameKey, size_t> seen{ };
    std::vector<Game> deduped{ };

    for ( size_t i = 0; i < games.size( ); i++ ) {
        auto& game = games[i];
        auto key = utils::get_game_identity_key( game );

        if ( key.kind == GameKeyKind::INVALID ) continue;

        if ( seen.contains( key ) ) {
            SPDLOG_INFO( "[Detection] {} has been seen already! removing duplicate.", key.value );
            auto& target_paths = deduped[seen[key]].save_paths;
            for ( auto& path : game.save_paths ) {
                bool skip = false;
                std::erase_if( target_paths, [&]( const fs::path& existing ) {
                    auto rel = path.lexically_relative( existing );
                    if ( !rel.empty( ) && rel.native( )[0] != '.' )
                        return true; // existing is a parent of path, drop it
                    auto rel2 = existing.lexically_relative( path );
                    if ( !rel2.empty( ) && rel2.native( )[0] != '.' )
                        skip = true; // path is a parent of existing, don't add path
                    return false;
                } );
                if ( !skip && std::ranges::find( target_paths, path ) == target_paths.end( ) ) {
                    target_paths.push_back( path );
                }
            }
        } else {
            deduped.push_back( game );
            seen[key] = deduped.size( ) - 1;
        }
    }
    return deduped;
}
