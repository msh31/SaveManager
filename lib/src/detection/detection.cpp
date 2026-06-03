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

void Detection::add_game(
    std::expected<std::vector<Game>, DetectionError> result, const std::string& platform, DetectionResult& d_result ) {
    if ( result ) {
        std::unique_lock<std::shared_mutex> lock( d_result.d_mutex );
        auto&                               v = result.value( );
        d_result.games.insert( d_result.games.end( ), v.begin( ), v.end( ) );
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
    const fs::path& compatdata, Detection::DetectionResult& result, const CConfig& config,
    const Detectors& detectors ) {
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
                    "ubi", result );
            }

            if ( fs::exists( users_dir ) ) {
                for ( const auto& user : fs::directory_iterator( users_dir ) ) {
                    if ( user.path( ).filename( ) == "Public" ) continue;

                    if ( config.settings.ubi_enabled ) {
                        Detection::add_game(
                            detectors.ubisoft_detect.find_saves( user.path( ) / "Documents" ), "ubi", result );
                        Detection::add_game(
                            detectors.ubisoft_detect.find_anno_saves( user.path( ) / "Documents" ), "ubi", result );
                        Detection::add_game(
                            detectors.ubisoft_detect.find_anno_saves( user.path( ) / "AppData" / "Roaming" ), "ubi",
                            result );
                    }
                    if ( config.settings.rsg_enabled ) {
                        Detection::add_game(
                            detectors.rockstar_detect.find_saves( user.path( ) / "Documents" / "Rockstar Games" ),
                            "rsg", result );
                        Detection::add_game(
                            detectors.rockstar_detect.find_legacy_saves( user.path( ) / "Documents" ), "rsg", result );
                        Detection::add_game(
                            detectors.rockstar_detect.find_legacy_saves(
                                user.path( ) / "AppData" / "Local" / "Rockstar Games" ),
                            "rsg", result );
                    }
                    if ( config.settings.unreal_enabled ) {
                        Detection::add_game( detectors.unreal_detect.find_saves( user.path( ) ), "unreal", result );
                    }
                }
            }
        } catch ( const fs::filesystem_error& fse ) {
            SPDLOG_WARN( "scan_prefix_dir: skipping {}: {}", entry.path( ).string( ), fse.what( ) );
        }
    }
}

void Detection::find_saves( CConfig& config, DetectionResult& d_result, std::optional<CLudusaviParser>& parser ) {
    Detectors detectors;
    auto      libraries = SteamHelper::get_library_folders( );

    if ( parser.has_value( ) ) {
        for ( const auto& library : libraries ) {
            for ( const auto& entry :
                  fs::directory_iterator( library / "steamapps", fs::directory_options::skip_permission_denied ) ) {
                if ( entry.is_directory( ) ) continue;
                if ( entry.path( ).extension( ) != ".acf" ) continue;

                auto manifest = SteamHelper::parse_app_manifest( entry.path( ) );
                if ( !manifest ) continue;

                auto parser_paths = parser->get_save_paths( manifest->appid );
                if ( parser_paths.empty( ) ) continue;

                for ( auto& path : parser_paths ) {
                    auto str = path.unresolved_path.string( );

                    auto pos = str.find( "<root>" );
                    if ( pos != std::string::npos ) {
                        str.replace( pos, std::string( "<root>" ).length( ), ( library / "steamapps" ).string( ) );
                    }

                    if ( !fs::exists( str ) ) continue;

                    Game game;
                    game.appid            = manifest->appid;
                    game.game_name        = manifest->name;
                    game.save_path        = str;
                    game.type             = PlatformType::GENERIC;
                    game.show_parent_path = true; // verify

                    std::scoped_lock lock( d_result.d_mutex );
                    d_result.games.emplace_back( game );
                }
            }
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
        Detection::add_game( plugins.find_saves( ), "Custom", d_result );
    }
    if ( plugin_count > 0 ) SPDLOG_INFO( "Loaded {} plugins!", plugin_count );

    // TODO: move this
    Detection::add_game( detectors.minecraft_detect.find_saves( ), "minecraft", d_result );

#ifdef __linux__
    // steam
    for ( const auto& library : libraries ) {
        fs::path compatdata = library / "steamapps/compatdata";
        if ( !fs::exists( compatdata ) ) continue;

        scan_prefix_dir( compatdata, d_result, config, detectors );
    }

    // lutris
    if ( fs::exists( paths::lutris_dir( ) ) ) {
        scan_prefix_dir( paths::lutris_dir( ), d_result, config, detectors );
    }

    // heroic
    fs::path heroic_dir = paths::heroic_dir( ) / "Prefixes/default";

    if ( fs::exists( heroic_dir ) ) {
        scan_prefix_dir( heroic_dir, d_result, config, detectors );
    }
#endif

#ifdef _WIN32
    if ( config.settings.ubi_enabled ) {
        Detection::add_game(
            detectors.ubisoft_detect.find_saves( "C:\\Program Files (x86)\\Ubisoft\\Ubisoft Game Launcher\\savegames" ),
            "ubi", d_result );
        Detection::add_game( detectors.ubisoft_detect.find_anno_saves( paths::documents_dir( ) ), "ubi", d_result );
        Detection::add_game(
            detectors.ubisoft_detect.find_anno_saves( paths::home_dir( ) / "AppData" / "Roaming" ), "ubi", d_result );
    }

    if ( config.settings.rsg_enabled ) {
        Detection::add_game(
            detectors.rockstar_detect.find_saves( paths::documents_dir( ) / "Rockstar Games" ), "rsg", d_result );
        Detection::add_game( detectors.rockstar_detect.find_legacy_saves( paths::documents_dir( ) ), "rsg", d_result );
        Detection::add_game(
            detectors.rockstar_detect.find_legacy_saves( paths::home_dir( ) / "AppData" / "Local" / "Rockstar Games" ),
            "rsg", d_result );
    }
    if ( config.settings.unreal_enabled ) {
        Detection::add_game( detectors.unreal_detect.find_saves( paths::home_dir( ) ), "unreal", d_result );
    }
    // Detection::add_game(detectors.custom_detect.find_saves(paths::home_dir()), "custom", d_result);
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
            "unreal", d_result );
        Detection::add_game(
            detectors.unreal_detect.find_saves( paths::heroic_dir( ) / "Prefixes" ), "unreal", d_result );
    }
    // Detection::add_game(detectors.custom_detect.find_saves(paths::home_dir()), "custom", result);
#endif

    std::unique_lock<std::shared_mutex> lock( d_result.d_mutex );
    if ( d_result.games.empty( ) ) {
        SPDLOG_ERROR( "No savegames found!" );
    }

    std::erase_if( d_result.games, []( const Game& game ) { return Blacklist::is_blacklisted( game.game_name ); } );
    std::erase_if( d_result.games, []( const Game& game ) {
        return !fs::is_directory( game.save_path ) || fs::is_empty( game.save_path );
    } );
}
