#include "minecraft.hpp"
#include "utils/paths.hpp"

std::expected<std::vector<Game>, SMError> CMinecraftDetector::find( ) {
    std::vector<Game> games;

    auto append = [&]( std::vector<Game> result ) { games.insert( games.end( ), result.begin( ), result.end( ) ); };

    append( scan_official( ) );
    append( scan_modrinth( ) );
    append( scan_prism( ) );
#if defined( __linux__ )
    append( scan_multimc( ) );
#endif
    append( scan_curseforge( ) );
    return games;
}

std::vector<Game> CMinecraftDetector::scan_official( ) const {
#if defined( __linux__ )
    fs::path game_path = paths::home_dir( ) / ".minecraft";
#elif defined( __APPLE__ )
    fs::path game_path = paths::home_dir( ) / "Library" / "Application Support" / "minecraft";
#elif defined( _WIN32 )
    fs::path game_path = paths::home_dir( ) / "AppData" / "Roaming" / ".minecraft";
#endif
    if ( !fs::exists( game_path ) ) return { };
    if ( !fs::is_directory( game_path ) ) return { };

    Game entry;
    entry.type = PlatformType::MINECRAFT;
    entry.game_name = "Minecraft (Official)";
    entry.appid = "N/A";
    entry.launcher = LauncherType::OFFICIAL;

    for ( const auto& game :
          fs::directory_iterator( game_path, std::filesystem::directory_options::skip_permission_denied ) ) {
        fs::path saves_folder = game.path( );
        if ( saves_folder.filename( ).string( ) != "saves" ) continue;

        for ( const auto& world :
              fs::directory_iterator( saves_folder, std::filesystem::directory_options::skip_permission_denied ) ) {
            if ( world.path( ).empty( ) ) continue;
            entry.save_paths.push_back( world.path( ) );
        }
    }

    if ( entry.save_paths.empty( ) ) return { };
    return { entry };
}

std::string_view CMinecraftDetector::name( ) const { return "Minecraft"; }

// private

std::vector<Game> CMinecraftDetector::scan_modrinth( ) const {
    std::vector<fs::path> modrinth_paths = { };
#if defined( __linux__ )
    auto mp1 = paths::home_dir( ) / ".local" / "share" / "ModrinthApp" / "profiles";
    auto mp2 = paths::home_dir( ) / ".var" / "app" / "com.modrinth.ModrinthApp" / "data" / "ModrinthApp" / "profiles";
    modrinth_paths.emplace_back( mp1 );
    modrinth_paths.emplace_back( mp2 );
#elif defined( __APPLE__ )
    modrinth_paths.emplace_back( paths::home_dir( ) / "Library" / "Application Support" / "ModrinthApp" / "profiles" );
#elif defined( _WIN32 )
    modrinth_paths.emplace_back( paths::home_dir( ) / "AppData" / "Roaming" / "ModrinthApp" / "profiles" );
#endif
    if ( modrinth_paths.empty( ) ) return { }; // failure

    Game entry;
    entry.type = PlatformType::MINECRAFT;
    entry.game_name = "Minecraft (Modrinth)";
    entry.appid = "N/A";
    entry.launcher = LauncherType::MODRINTH;

    for ( const auto& path : modrinth_paths ) {
        if ( !fs::exists( path ) ) continue;
        for ( const auto& game :
              fs::directory_iterator( path, std::filesystem::directory_options::skip_permission_denied ) ) {
            fs::path game_folder = game.path( );
            if ( !fs::is_directory( game_folder ) ) continue;

            for ( const auto& profile :
                  fs::directory_iterator( game_folder, std::filesystem::directory_options::skip_permission_denied ) ) {
                if ( profile.path( ).filename( ).string( ) != "saves" ) continue;
                for ( const auto& world : fs::directory_iterator(
                          profile.path( ), std::filesystem::directory_options::skip_permission_denied ) ) {
                    if ( world.path( ).empty( ) ) continue;
                    entry.save_paths.push_back( world.path( ) );
                }
            }
        }
    }

    if ( entry.save_paths.empty( ) ) return { };
    return { entry };
}

std::vector<Game> CMinecraftDetector::scan_curseforge( ) const {
#if defined( __linux__ ) || defined( __APPLE__ )
    fs::path curse_path = paths::home_dir( ) / "Documents" / "curseforge" / "minecraft" / "Instances";
#elif defined( _WIN32 )
    fs::path curse_path = paths::home_dir( ) / "Documents" / "curseforge" / "minecraft" / "Instances";
#endif
    if ( !fs::exists( curse_path ) ) return { };
    if ( !fs::is_directory( curse_path ) ) return { };

    Game entry;
    entry.type = PlatformType::MINECRAFT;
    entry.game_name = "Minecraft (CurseForge)";
    entry.appid = "N/A";
    entry.launcher = LauncherType::CURSEFORGE;

    for ( const auto& game :
          fs::directory_iterator( curse_path, std::filesystem::directory_options::skip_permission_denied ) ) {
        fs::path game_folder = game.path( );
        if ( !fs::is_directory( game_folder ) ) continue;

        for ( const auto& profile :
              fs::directory_iterator( game_folder, std::filesystem::directory_options::skip_permission_denied ) ) {
            if ( profile.path( ).filename( ).string( ) != "saves" ) continue;

            for ( const auto& world : fs::directory_iterator(
                      profile.path( ), std::filesystem::directory_options::skip_permission_denied ) ) {
                if ( world.path( ).empty( ) ) continue;
                entry.save_paths.push_back( world.path( ) );
            }
        }
    }

    if ( entry.save_paths.empty( ) ) return { };
    return { entry };
}

std::vector<Game> CMinecraftDetector::scan_prism( ) const {
#if defined( __linux__ )
    fs::path prism_path = paths::home_dir( ) / ".var" / "app" / "org.prismlauncher.PrismLauncher" / "data" /
                          "PrismLauncher" / "instances";
#elif defined( __APPLE__ )
    fs::path prism_path = paths::home_dir( ) / "Library" / "Application Support" / "PrismLauncher" / "instances";
#elif defined( _WIN32 )
    fs::path prism_path = paths::home_dir( ) / "AppData" / "Roaming" / "PrismLauncher" / "instances";
#endif
    if ( !fs::exists( prism_path ) ) return { };

    Game entry;
    entry.type = PlatformType::MINECRAFT;
    entry.game_name = "Minecraft (Prism)";
    entry.appid = "N/A";
    entry.launcher = LauncherType::PRISM;

    for ( const auto& game :
          fs::directory_iterator( prism_path, std::filesystem::directory_options::skip_permission_denied ) ) {
        fs::path game_folder = game.path( );
        if ( !fs::is_directory( game_folder ) ) continue;

        for ( const auto& folder :
              fs::directory_iterator( game_folder, std::filesystem::directory_options::skip_permission_denied ) ) {
            if ( folder.path( ).filename( ).string( ) != "minecraft" ) continue;
            if ( !fs::is_directory( folder ) ) continue;

            for ( const auto& profile :
                  fs::directory_iterator( folder, std::filesystem::directory_options::skip_permission_denied ) ) {
                if ( profile.path( ).filename( ).string( ) != "saves" ) continue;
                if ( !fs::is_directory( profile ) ) continue;

                for ( const auto& world : fs::directory_iterator(
                          profile.path( ), std::filesystem::directory_options::skip_permission_denied ) ) {
                    if ( world.path( ).empty( ) ) continue;
                    entry.save_paths.push_back( world.path( ) );
                }
            }
        }
    }

    if ( entry.save_paths.empty( ) ) return { };
    return { entry };
}

std::vector<Game> CMinecraftDetector::scan_multimc( ) const {
    fs::path multimc_path = paths::home_dir( ) / ".local" / "share" / "multimc" / "instances";

    if ( !fs::exists( multimc_path ) ) return { };

    Game entry;
    entry.type = PlatformType::MINECRAFT;
    entry.game_name = "Minecraft (MultiMC)";
    entry.appid = "N/A";
    entry.launcher = LauncherType::MULTIMC;

    for ( const auto& game :
          fs::directory_iterator( multimc_path, std::filesystem::directory_options::skip_permission_denied ) ) {
        fs::path game_folder = game.path( );
        if ( !fs::is_directory( game_folder ) ) continue;

        for ( const auto& profile :
              fs::directory_iterator( game_folder, std::filesystem::directory_options::skip_permission_denied ) ) {
            if ( profile.path( ).filename( ).string( ) != "saves" ) continue;

            for ( const auto& world : fs::directory_iterator(
                      profile.path( ), std::filesystem::directory_options::skip_permission_denied ) ) {
                if ( world.path( ).empty( ) ) continue;
                entry.save_paths.push_back( world.path( ) );
            }
        }
    }

    if ( entry.save_paths.empty( ) ) return { };
    return { entry };
}
