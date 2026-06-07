#include "detection/detection.hpp"
#include "config/config.hpp"
#include "plugin/plugin.hpp"

#include "utils/blacklist/blacklist.hpp"
#include "utils/paths.hpp"

#include <utils/steam/steam.hpp>

#include "detection/minecraft/minecraft.hpp"
#include "detection/rsg/rsg.hpp"
#include "detection/ubi/ubi.hpp"
#include "detection/unreal/unreal.hpp"
#include <utils/ludisavi_parser/ludusavi_parser.hpp>

struct Detectors {
        CRockstarDetector  rockstar_detect;
        CUbisoftDetector   ubisoft_detect;
        CUnrealDetector    unreal_detect;
        CMinecraftDetector minecraft_detect;
};

GameKey Detection::get_game_identity_key( const Game& game ) {
    if ( !game.appid.empty( ) ) return { GameKeyKind::STEAM_APPID, game.appid };

    // ubisoft
    if ( game.game_id.has_value( ) ) return { GameKeyKind::UBISOFT_ID, *game.game_id };

    // minecraft launchers
    if ( game.type == PlatformType::MINECRAFT ) return { GameKeyKind::MINECRAFT, game.game_name };

    if ( !game.game_name.empty( ) ) return { GameKeyKind::NAME, game.game_name };

    if ( !game.save_paths.empty( ) ) {
        SPDLOG_INFO( "save path hit: {}", game.game_name );
        return { GameKeyKind::PATH, weakly_canonical( game.save_paths[0] ).string( ) };
    }

    SPDLOG_ERROR( "Failed to get game identify key" );
    return { GameKeyKind::INVALID }; // caller must check this
}

void Detection::add_game(
    std::expected<std::vector<Game>, DetectionError> result, const std::string& platform, std::vector<Game>& games ) {

    if ( result ) {
        // std::unique_lock<std::shared_mutex> lock( games.d_mutex );
        auto& v = result.value( );
        games.insert( games.end( ), v.begin( ), v.end( ) );
    } else {
        switch ( result.error( ) ) {
        case DetectionError::PathNotFound:
            break;
        case DetectionError::PermissionDenied:
            SPDLOG_WARN( "{}: permission denied", platform );
            break;
        case DetectionError::NoSavesFound:
            SPDLOG_WARN( "{}: no saves found", platform );
            break;
        }
    }
}

void scan_prefix_dir(
    const fs::path& compatdata, std::vector<Game>& games, const CConfig& config, const Detectors& detectors ) {
    for ( const auto& entry : fs::directory_iterator( compatdata ) ) {
        try {
            fs::path prefix = entry.path( );
            if ( !fs::exists( prefix ) ) continue;

            fs::path drive_c   = fs::exists( prefix / "pfx" ) ? prefix / "pfx/drive_c" : prefix / "drive_c";
            fs::path users_dir = drive_c / "users";
            if ( config.settings.ubi_enabled ) {
                Detection::add_game(
                    detectors.ubisoft_detect.find_saves(
                        drive_c / "Program Files (x86)" / "Ubisoft" / "Ubisoft Game Launcher" / "savegames" ),
                    "ubi", games );
            }

            if ( fs::exists( users_dir ) ) {
                for ( const auto& user : fs::directory_iterator( users_dir ) ) {
                    if ( user.path( ).filename( ) == "Public" ) continue;

                    if ( config.settings.ubi_enabled ) {
                        Detection::add_game(
                            detectors.ubisoft_detect.find_saves( user.path( ) / "Documents" ), "ubi", games );
                        Detection::add_game(
                            detectors.ubisoft_detect.find_anno_saves( user.path( ) / "Documents" ), "ubi", games );
                        Detection::add_game(
                            detectors.ubisoft_detect.find_anno_saves( user.path( ) / "AppData" / "Roaming" ), "ubi",
                            games );
                    }
                    if ( config.settings.rsg_enabled ) {
                        Detection::add_game(
                            detectors.rockstar_detect.find_saves( user.path( ) / "Documents" / "Rockstar Games" ),
                            "rsg", games );
                        Detection::add_game(
                            detectors.rockstar_detect.find_legacy_saves( user.path( ) / "Documents" ), "rsg", games );
                        Detection::add_game(
                            detectors.rockstar_detect.find_legacy_saves(
                                user.path( ) / "AppData" / "Local" / "Rockstar Games" ),
                            "rsg", games );
                    }
                    if ( config.settings.unreal_enabled ) {
                        Detection::add_game( detectors.unreal_detect.find_saves( user.path( ) ), "unreal", games );
                    }
                }
            }
        } catch ( const fs::filesystem_error& fse ) {
            SPDLOG_WARN( "scan_prefix_dir: skipping {}: {}", entry.path( ).string( ), fse.what( ) );
        }
    }
}

void Detection::find_saves( CConfig& config, std::vector<Game>& games ) {
    Detectors detectors;
    // CLudusaviParser parser;
    //
    // Detection::find_saves_ludusavi( config, games, parser );

    // cool lua support
    int plugin_count = 0;
    for ( const auto& plugin : fs::recursive_directory_iterator(
              paths::plugin_dir( ),
              fs::directory_options::skip_permission_denied | fs::directory_options::follow_directory_symlink ) ) {
        if ( plugin.path( ).extension( ) != ".lua" ) continue;
        if ( !fs::is_regular_file( plugin ) ) continue;
        plugin_count++;

        CPlugin plugins( plugin );
        Detection::add_game( plugins.find_saves( ), "Custom", games );
    }
    if ( plugin_count > 0 ) SPDLOG_INFO( "Loaded {} plugins!", plugin_count );

    Detection::add_game( detectors.minecraft_detect.find_saves( ), "minecraft", games );

#ifdef __linux__
    auto libraries = SteamHelper::get_library_folders( );

    // steam
    for ( const auto& library : libraries ) {
        fs::path compatdata = library / "steamapps/compatdata";
        if ( !fs::exists( compatdata ) ) continue;

        scan_prefix_dir( compatdata, games, config, detectors );
    }

    // lutris
    if ( fs::exists( paths::lutris_dir( ) ) ) {
        scan_prefix_dir( paths::lutris_dir( ), games, config, detectors );
    }

    // heroic
    fs::path heroic_dir = paths::heroic_dir( ) / "Prefixes/default";

    if ( fs::exists( heroic_dir ) ) {
        scan_prefix_dir( heroic_dir, games, config, detectors );
    }
#endif

#ifdef _WIN32
    if ( config.settings.ubi_enabled ) {
        Detection::add_game(
            detectors.ubisoft_detect.find_saves( "C:\\Program Files (x86)\\Ubisoft\\Ubisoft Game Launcher\\savegames" ),
            "ubi", games );
        Detection::add_game( detectors.ubisoft_detect.find_anno_saves( paths::documents_dir( ) ), "ubi", games );
        Detection::add_game(
            detectors.ubisoft_detect.find_anno_saves( paths::home_dir( ) / "AppData" / "Roaming" ), "ubi", games );
    }

    if ( config.settings.rsg_enabled ) {
        Detection::add_game(
            detectors.rockstar_detect.find_saves( paths::documents_dir( ) / "Rockstar Games" ), "rsg", games );
        Detection::add_game( detectors.rockstar_detect.find_legacy_saves( paths::documents_dir( ) ), "rsg", games );
        Detection::add_game(
            detectors.rockstar_detect.find_legacy_saves( paths::home_dir( ) / "AppData" / "Local" / "Rockstar Games" ),
            "rsg", games );
    }
    if ( config.settings.unreal_enabled ) {
        Detection::add_game( detectors.unreal_detect.find_saves( paths::home_dir( ) ), "unreal", games );
    }
#endif

#ifdef __APPLE__
    if ( config.settings.ubi_enabled ) {
        // ignored
    }

    if ( config.settings.rsg_enabled ) {
        // ignored
    }
    if ( config.settings.unreal_enabled ) {
        Detection::add_game(
            detectors.unreal_detect.find_saves(
                paths::home_dir( ) / "Library" / "Application Support", CUnrealDetector::ScanMode::Native ),
            "unreal", games );
        Detection::add_game( detectors.unreal_detect.find_saves( paths::heroic_dir( ) / "Prefixes" ), "unreal", games );
    }
#endif

    {
        // std::unique_lock<std::shared_mutex> lock( games.d_mutex );
        if ( games.empty( ) ) {
            SPDLOG_ERROR( "No savegames found!" );
        }

        std::map<GameKey, size_t> seen;
        std::vector<size_t>       to_remove;
        for ( size_t i = 0; i < games.size( ); i++ ) {
            auto& game = games[i];
            auto  key  = get_game_identity_key( game );

            if ( seen.contains( key ) ) {
                auto& existing = games[seen[key]];
                existing.save_paths.insert(
                    existing.save_paths.end( ), game.save_paths.begin( ), game.save_paths.end( ) );
                to_remove.push_back( i );
            } else {
                seen[key] = i;
            }
        }

        for ( int i = to_remove.size( ) - 1; i >= 0; i-- ) {
            games.erase( games.begin( ) + to_remove[i] );
        }

        std::erase_if( games, []( const Game& game ) { return Blacklist::is_blacklisted( game.game_name ); } );
        std::erase_if( games, []( const Game& game ) {
            return std::ranges::none_of(
                game.save_paths, []( const fs::path& p ) { return fs::is_directory( p ) && !fs::is_empty( p ); } );
        } );
    }
}

void Detection::find_saves_ludusavi(
    CConfig& config, std::vector<Game>& games, std::shared_ptr<CLudusaviParser>& parser ) {
    if ( parser == nullptr ) return;

    auto manifest = SteamHelper::get_app_manifests( );
    if ( manifest.empty( ) ) return;

    std::unordered_set<std::string> seen;
    {
        // std::shared_lock<std::shared_mutex> lock( games.d_mutex );
        for ( const auto& g : games ) {
            seen.insert( g.appid );
        }
    }

    std::vector<Game> discovered;

    for ( const auto& entry : manifest ) {
        const SteamManifest& sm        = entry.second;
        std::string          appid_str = std::to_string( sm.appid );
        if ( seen.contains( appid_str ) ) continue;

        CLudusaviParser::ResolveContext ctx;
        ctx.install_dir = sm.library_dir / "steamapps" / "common" / sm.install_dir;
        fs::path prefix = sm.library_dir / "steamapps" / "compatdata" / appid_str / "pfx";
        if ( fs::exists( prefix ) ) ctx.proton_prefix = prefix;

        auto parser_paths = parser->get_save_paths( sm.appid, ctx );
        if ( parser_paths.empty( ) ) continue;

        for ( auto& path : parser_paths ) {
            const auto& str = path.resolved_path;
            if ( !fs::exists( str ) ) continue;
            if ( fs::is_regular_file( str ) && fs::file_size( str ) == 0 ) continue;

            Game game;
            game.appid     = appid_str;
            game.game_name = sm.name;
            // game.save_path        = str;
            game.save_paths.emplace_back( str );
            game.type             = PlatformType::GENERIC;
            game.show_parent_path = true; // verify
            // SPDLOG_INFO( "ludusavi: {} -> {}", sm.name, str.string( ) );

            discovered.emplace_back( std::move( game ) );
        }

        seen.insert( appid_str );
    }

    if ( discovered.empty( ) ) return;

    // std::unique_lock<std::shared_mutex> lock( games.d_mutex );
    games.insert(
        games.end( ), std::make_move_iterator( discovered.begin( ) ), std::make_move_iterator( discovered.end( ) ) );
}
