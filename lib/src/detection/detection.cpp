#include "detection/detection.hpp"
#include "config/config.hpp"
#include "plugin/plugin.hpp"

#include "utils/blacklist/blacklist.hpp"
#include "utils/paths.hpp"
#include <utils/utils.hpp>

#include <utils/steam/steam.hpp>

#include "detection/minecraft/minecraft.hpp"
#include "detection/rsg/rsg.hpp"
#include "detection/ubi/ubi.hpp"
#include "detection/unreal/unreal.hpp"

void Detection::find_saves( CConfig& config, std::vector<Game>& games ) {
    std::vector<std::unique_ptr<IDetector>> detectors;

#ifdef _WIN32
    detectors.emplace_back( std::make_unique<CUbisoftDetector>( ) );
    detectors.emplace_back( std::make_unique<CRockstarDetector>( ) );
    detectors.emplace_back( std::make_unique<CUnrealDetector>( ) );
#endif

#ifdef __linux__
// TODO: CWinePrefixDetector implementation
#endif

#ifdef __APPLE__
    detectors.emplace_back( std::make_unique<CUnrealDetector>( ) );
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
        // Detection::add_game( plugins.find_saves( ), "Custom", games );
    }
    if ( plugin_count > 0 ) SPDLOG_INFO( "Loaded {} plugins!", plugin_count );

    // Detection::add_game( detectors.minecraft_detect.find( ), "minecraft", games );

#ifdef __linux__
    // std::vector<std::future<std::vector<Game>>> steam_futures;
    // std::future<std::vector<Game>>              lutris_future;
    // std::future<std::vector<Game>>              heroic_future;
    //
    // auto libraries = SteamHelper::get_library_folders( );
    //
    // // steam
    // for ( const auto& library : libraries ) {
    //     fs::path compatdata = library / "steamapps/compatdata";
    //     if ( !fs::exists( compatdata ) ) continue;
    //
    //     steam_futures.push_back(
    //         std::async( std::launch::async, [compatdata, config, detectors]( ) -> std::vector<Game> {
    //             return scan_prefix_dir( compatdata, config, detectors );
    //         } ) );
    // }
    //
    // // lutris
    // auto lutris_dir = paths::lutris_dir( );
    // if ( fs::exists( lutris_dir ) ) {
    //     lutris_future = std::async( std::launch::async, [lutris_dir, config, detectors]( ) ->
    //     std::vector<Game> {
    //         return scan_prefix_dir( lutris_dir, config, detectors );
    //     } );
    // }
    //
    // // heroic
    // fs::path heroic_dir = paths::heroic_dir( ) / "Prefixes/default";
    // if ( fs::exists( heroic_dir ) ) {
    //     heroic_future = std::async( std::launch::async, [heroic_dir, config, detectors]( ) ->
    //     std::vector<Game> {
    //         return scan_prefix_dir( heroic_dir, config, detectors );
    //     } );
    // }
    //
    // // inserting the results from all scans
    // if ( lutris_future.valid( ) ) {
    //     auto lutris_result = lutris_future.get( );
    //     games.insert( games.end( ), lutris_result.begin( ), lutris_result.end( ) );
    // }
    // if ( heroic_future.valid( ) ) {
    //     auto heroic_result = heroic_future.get( );
    //     games.insert( games.end( ), heroic_result.begin( ), heroic_result.end( ) );
    // }
    // for ( auto& future : steam_futures ) {
    //     if ( future.valid( ) ) {
    //
    //         auto result = future.get( );
    //         games.insert( games.end( ), result.begin( ), result.end( ) );
    //     }
    // }
#endif

#ifdef _WIN32
    // std::future<std::vector<Game>> ubisoft_future;
    // std::future<std::vector<Game>> rockstar_future;
    // std::future<std::vector<Game>> unreal_future;
    //
    // if ( config.settings.ubi_enabled ) {
    //     ubisoft_future = std::async( std::launch::async, [detectors, config]( ) -> std::vector<Game> {
    //         std::vector<Game> result;
    //         Detection::add_game(
    //             detectors.ubisoft_detect.find_saves(
    //                 "C:\\Program Files (x86)\\Ubisoft\\Ubisoft Game Launcher\\savegames" ),
    //             "ubi", result );
    //         Detection::add_game( detectors.ubisoft_detect.find_anno_saves( paths::documents_dir( ) ), "ubi", result
    //         ); Detection::add_game(
    //             detectors.ubisoft_detect.find_anno_saves( paths::home_dir( ) / "AppData" / "Roaming" ), "ubi", result
    //             );
    //
    //         return result;
    //     } );
    // }
    //
    // if ( config.settings.rsg_enabled ) {
    //     rockstar_future = std::async( std::launch::async, [detectors, config]( ) -> std::vector<Game> {
    //         std::vector<Game> result;
    //         Detection::add_game(
    //             detectors.rockstar_detect.find_saves( paths::documents_dir( ) / "Rockstar Games" ), "rsg", result );
    //         Detection::add_game(
    //             detectors.rockstar_detect.find_legacy_saves( paths::documents_dir( ) ), "rsg", result );
    //         Detection::add_game(
    //             detectors.rockstar_detect.find_legacy_saves(
    //                 paths::home_dir( ) / "AppData" / "Local" / "Rockstar Games" ),
    //             "rsg", result );
    //
    //         return result;
    //     } );
    // }
    // if ( config.settings.unreal_enabled ) {
    //     unreal_future = std::async( std::launch::async, [detectors, config]( ) -> std::vector<Game> {
    //         std::vector<Game> result;
    //         Detection::add_game( detectors.unreal_detect.find_saves( paths::home_dir( ) ), "unreal", result );
    //         return result;
    //     } );
    // }
    //
    // if ( ubisoft_future.valid( ) ) {
    //     auto ubisoft_result = ubisoft_future.get( );
    //     games.insert( games.end( ), ubisoft_result.begin( ), ubisoft_result.end( ) );
    // }
    // if ( rockstar_future.valid( ) ) {
    //     auto rockstar_result = rockstar_future.get( );
    //     games.insert( games.end( ), rockstar_result.begin( ), rockstar_result.end( ) );
    // }
    // if ( unreal_future.valid( ) ) {
    //     auto unreal_result = unreal_future.get( );
    //     games.insert( games.end( ), unreal_result.begin( ), unreal_result.end( ) );
    // }
#endif

#ifdef __APPLE__
    // std::future<std::vector<Game>> unreal_future;
    //
    // if ( config.settings.unreal_enabled ) {
    //     unreal_future = std::async( std::launch::async, [detectors, config]( ) -> std::vector<Game> {
    //         std::vector<Game> result;
    //         Detection::add_game(
    //             detectors.unreal_detect.find_saves(
    //                 paths::home_dir( ) / "Library" / "Application Support", CUnrealDetector::ScanMode::Native ),
    //             "unreal", result );
    //         Detection::add_game(
    //             detectors.unreal_detect.find_saves( paths::heroic_dir( ) / "Prefixes" ), "unreal", result );
    //         return result;
    //     } );
    // }
    //
    // if ( unreal_future.valid( ) ) {
    //     auto unreal_result = unreal_future.get( );
    //     games.insert( games.end( ), unreal_result.begin( ), unreal_result.end( ) );
    // }
#endif

    if ( games.empty( ) ) {
        SPDLOG_ERROR( "No savegames found!" );
    }

    std::map<GameKey, size_t> seen;
    std::vector<Game>         deduped;
    for ( size_t i = 0; i < games.size( ); i++ ) {
        auto& game = games[i];
        auto  key  = utils::get_game_identity_key( game );

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
}

// void Detection::add_game(
//     std::expected<std::vector<Game>, SMError> result, const std::string& platform, std::vector<Game>& games ) {
//
//     if ( result ) {
//         auto& v = result.value( );
//         games.insert( games.end( ), v.begin( ), v.end( ) );
//     } else {
//         switch ( result.error( ) ) {
//         case SMError::PATH_NOT_FOUND:
//             break;
//         case SMError::PERMISSION_DENIED:
//             SPDLOG_WARN( "{}: permission denied", platform );
//             break;
//         case SMError::NO_SAVES_FOUND:
//             SPDLOG_WARN( "{}: no saves found", platform );
//             break;
//         case SMError::CONFIG_PARSING_ERROR:
//             SPDLOG_ERROR( "Failed to parse config!" );
//             break;
//         case SMError::CONFIG_LOAD_ERROR:
//             SPDLOG_ERROR( "Failed to load config!" );
//             break;
//         case SMError::CONNECTION_FAILED:
//             SPDLOG_ERROR( "Failed to communicate with server!" );
//             break;
//         case SMError::DOWNLOAD_FAILED:
//             SPDLOG_ERROR( "Failed to download file!" );
//             break;
//         case SMError::PLUGIN_LOAD_ERROR:
//             SPDLOG_ERROR( "Failed to load plugin!" );
//             break;
//         }
//     }
// }

// #if defined( __linux__ )
// std::vector<Game> scan_prefix_dir( const fs::path& compatdata, const CConfig& config, const Detectors& detectors ) {
//     std::vector<Game> games;
//
//     for ( const auto& entry : fs::directory_iterator( compatdata ) ) {
//         try {
//             fs::path prefix = entry.path( );
//             if ( !fs::exists( prefix ) ) continue;
//
//             fs::path drive_c   = fs::exists( prefix / "pfx" ) ? prefix / "pfx/drive_c" : prefix / "drive_c";
//             fs::path users_dir = drive_c / "users";
//             if ( config.settings.ubi_enabled ) {
//                 Detection::add_game(
//                     detectors.ubisoft_detect.find_saves(
//                         drive_c / "Program Files (x86)" / "Ubisoft" / "Ubisoft Game Launcher" / "savegames" ),
//                     "ubi", games );
//             }
//
//             if ( fs::exists( users_dir ) ) {
//                 for ( const auto& user : fs::directory_iterator( users_dir ) ) {
//                     if ( user.path( ).filename( ) == "Public" ) continue;
//
//                     if ( config.settings.ubi_enabled ) {
//                         Detection::add_game(
//                             detectors.ubisoft_detect.find_saves( user.path( ) / "Documents" ), "ubi", games );
//                         Detection::add_game(
//                             detectors.ubisoft_detect.find_anno_saves( user.path( ) / "Documents" ), "ubi", games );
//                         Detection::add_game(
//                             detectors.ubisoft_detect.find_anno_saves( user.path( ) / "AppData" / "Roaming" ), "ubi",
//                             games );
//                     }
//                     if ( config.settings.rsg_enabled ) {
//                         Detection::add_game(
//                             detectors.rockstar_detect.find_saves( user.path( ) / "Documents" / "Rockstar Games" ),
//                             "rsg", games );
//                         Detection::add_game(
//                             detectors.rockstar_detect.find_legacy_saves( user.path( ) / "Documents" ), "rsg", games
//                             );
//                         Detection::add_game(
//                             detectors.rockstar_detect.find_legacy_saves(
//                                 user.path( ) / "AppData" / "Local" / "Rockstar Games" ),
//                             "rsg", games );
//                     }
//                     if ( config.settings.unreal_enabled ) {
//                         Detection::add_game( detectors.unreal_detect.find_saves( user.path( ) ), "unreal", games );
//                     }
//                 }
//             }
//         } catch ( const fs::filesystem_error& fse ) {
//             SPDLOG_WARN( "scan_prefix_dir: skipping {}: {}", entry.path( ).string( ), fse.what( ) );
//         }
//     }
//     return games;
// }
// #endif
