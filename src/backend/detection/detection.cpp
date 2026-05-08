#include "detection.hpp"
#include "backend/plugin/plugin.hpp"
#include "backend/config/config.hpp"
#include "backend/utils/blacklist/blacklist.hpp"
#include "backend/utils/paths.hpp"
#include "backend/logger/logger.hpp"
#include "backend/utils/utils.hpp"

#include <backend/utils/steam/steam.hpp>

#include "backend/detection/rsg/rsg.hpp"
#include "backend/detection/ubi/ubi.hpp"
#include "backend/detection/unreal/unreal.hpp"
#include "backend/detection/minecraft/minecraft.hpp"

// #include "backend/detection/custom/custom.hpp"

struct Detectors {
   RockstarDetector rockstar_detect; 
   UbisoftDetector ubisoft_detect; 
   UnrealDetector unreal_detect; 
   MinecraftDetector minecraft_detect; 
   // CustomDetector custom_detect; 
};



void Detection::add_game(std::expected<std::vector<Game>, DetectionError> result, const std::string& platform, DetectionResult& d_result) {
    if (result) {
        std::unique_lock<std::shared_mutex> lock(d_result.d_mutex);
        auto& v = result.value();
        d_result.games.insert(d_result.games.end(), v.begin(), v.end());
    } else {
        switch(result.error()) {
            case DetectionError::PathNotFound:
                break;
            case DetectionError::PermissionDenied:
                get_logger().warning("{}: permission denied", platform);
                break;
            case DetectionError::NoSavesFound:
                get_logger().warning("{}: no saves found", platform);
                break;
        }
    }
}

//try catch here is band-aid fix
//an exception escapes somewhere and causes a crash
//more investigation needed
void scan_prefix_dir(const fs::path& compatdata, Detection::DetectionResult& result, const Config& config, const Detectors& detectors) {
    try {
        for (const auto& entry : fs::directory_iterator(compatdata)) {
            fs::path prefix = entry.path();
            if(!fs::exists(prefix)) {
                get_logger().warning("Prefix not found!");
                continue;
            }

            fs::path drive_c = fs::exists(prefix / "pfx") ? prefix / "pfx/drive_c" : prefix / "drive_c";
            fs::path users_dir = drive_c / "users";
            if (config.settings.ubi_enabled) {
                Detection::add_game(detectors.ubisoft_detect.find_saves(drive_c / "Program Files (x86)" / "Ubisoft" / "Ubisoft Game Launcher" / "savegames"), "ubi", result);
            }
            // Detection::add_game(detectors.custom_detect.find_saves(drive_c), "custom", result); //might be too broad

            if (fs::exists(users_dir)) {
                for (const auto& user : fs::directory_iterator(users_dir)) {
                    if (user.path().filename() == "Public") continue;

                    if(config.settings.ubi_enabled) {
                        Detection::add_game(detectors.ubisoft_detect.find_saves(user.path() / "Documents"), "ubi", result);
                        Detection::add_game(detectors.ubisoft_detect.find_anno_saves(user.path() / "Documents"), "ubi", result);
                        Detection::add_game(detectors.ubisoft_detect.find_anno_saves(user.path() / "AppData" / "Roaming"), "ubi", result);
                    }
                    if (config.settings.rsg_enabled) {
                        Detection::add_game(detectors.rockstar_detect.find_saves(user.path() / "Documents" / "Rockstar Games"), "rsg", result);
                        Detection::add_game(detectors.rockstar_detect.find_legacy_saves(user.path() / "Documents"), "rsg", result);
                        Detection::add_game(detectors.rockstar_detect.find_legacy_saves(user.path() / "AppData" / "Local" / "Rockstar Games"), "rsg", result);
                    }
                    if (config.settings.unreal_enabled) {
                        Detection::add_game(detectors.unreal_detect.find_saves(user.path()), "unreal", result);
                    }
                }
            }
        }
    } catch(const std::filesystem::filesystem_error& fse) {
        get_logger().error("scan_prefix_dir: {}", fse.what());
    };
}

void Detection::find_saves(Config& config, DetectionResult& d_result) {
    Detectors detectors;

//cool lua support
    int plugin_count = 0;
    for (const auto& plugin : fs::directory_iterator(paths::plugin_dir(), fs::directory_options::skip_permission_denied)) {
        plugin_count++;
        // get_logger().debug("found: {}", plugin.path().string());
        if(plugin.path().extension() != ".lua") continue;
        if(!fs::is_regular_file(plugin)) continue;

        Plugin plugins(plugin);
        Detection::add_game(plugins.find_saves(), "Custom", d_result);
    }
    if(plugin_count > 0) get_logger().info("Loaded {} plugins!", plugin_count);

//TODO: move this & make it platform agnostic
    Detection::add_game(detectors.minecraft_detect.find_saves(), "minecraft", d_result);

#ifdef __linux__
    // steam
    auto libraries = SteamHelper::get_library_folders();
    for (const auto& library : libraries) {
        fs::path compatdata = library / "steamapps/compatdata";
        if (!fs::exists(compatdata)) continue;

        scan_prefix_dir(compatdata, d_result, config, detectors);
    }
    // lutris
    fs::path resolved_lutris = paths::lutris_dir();
    if (!config.settings.lutris_path.empty()) {
        if (fs::exists(config.settings.lutris_path)) {
            resolved_lutris = config.settings.lutris_path;
        } else {
            get_logger().warning("Configured Lutris path does not exist, falling back to defaults");
        }
    }

    if (fs::exists(resolved_lutris)) {
        if (config.settings.lutris_path.empty()) {
            config.settings.lutris_path = resolved_lutris.string();
        }

        scan_prefix_dir(resolved_lutris, d_result, config, detectors);
    }
    //heroic
    fs::path heroic_base = paths::heroic_dir();
    if (!config.settings.heroic_path.empty()) {
        if (fs::exists(config.settings.heroic_path)) {
            heroic_base = config.settings.heroic_path;
        } else {
            get_logger().warning("Configured Heroic path does not exist, falling back to defaults");
        }
    }
    fs::path heroic_dir = heroic_base / "Prefixes/default";

    if (fs::exists(heroic_dir)) {
        if (config.settings.heroic_path.empty()) {
            config.settings.heroic_path = paths::heroic_dir().string();
        }

        scan_prefix_dir(heroic_dir, d_result, config, detectors);
    } else {
        get_logger().warning("Heroic path does not exist!");
    }
#endif

#ifdef _WIN32
    if(config.settings.ubi_enabled) {
        Detection::add_game(detectors.ubisoft_detect.find_saves("C:\\Program Files (x86)\\Ubisoft\\Ubisoft Game Launcher\\savegames"), "ubi", d_result);
        Detection::add_game(detectors.ubisoft_detect.find_anno_saves(paths::documents_dir()), "ubi", d_result);
        Detection::add_game(detectors.ubisoft_detect.find_anno_saves(paths::home_dir() / "AppData" / "Roaming"), "ubi", d_result);
    }

    if(config.settings.rsg_enabled) {
        Detection::add_game(detectors.rockstar_detect.find_saves(paths::documents_dir() / "Rockstar Games"), "rsg", d_result);
        Detection::add_game(detectors.rockstar_detect.find_legacy_saves(paths::documents_dir()), "rsg", d_result);
        Detection::add_game(detectors.rockstar_detect.find_legacy_saves(paths::home_dir() / "AppData" / "Local" / "Rockstar Games"), "rsg", d_result);
    }
    if (config.settings.unreal_enabled) {
        Detection::add_game(detectors.unreal_detect.find_saves(paths::home_dir()), "unreal", d_result);
    }
    // Detection::add_game(detectors.custom_detect.find_saves(paths::home_dir()), "custom", d_result);
#endif

#ifdef __APPLE__
    if(config.settings.ubi_enabled) {
        //ignored
    }

    if(config.settings.rsg_enabled) {
        //ignored
    }
    if (config.settings.unreal_enabled) {
        Detection::add_game(detectors.unreal_detect.find_saves(paths::home_dir() / "Library" / "Application Support", UnrealDetector::ScanMode::Native), "unreal", d_result); 
        Detection::add_game(detectors.unreal_detect.find_saves(paths::heroic_dir() / "Prefixes"), "unreal", d_result); 
    }
    // Detection::add_game(detectors.custom_detect.find_saves(paths::home_dir()), "custom", result); 
#endif

    std::unique_lock<std::shared_mutex> lock(d_result.d_mutex);
    if (d_result.games.empty()) {
        get_logger().error("No savegames found!");
    }

    std::erase_if(d_result.games, [](const Game& game) {
        return Blacklist::is_blacklisted(game.game_name);
    });
    std::erase_if(d_result.games, [](const Game& game) {
        return !fs::is_directory(game.save_path) || fs::is_empty(game.save_path);
    });
}

std::vector<std::vector<int>> Detection::DetectionResult::get_grouped() const {
    std::unordered_map<std::string, size_t> key_to_group;
    std::vector<std::vector<int>> groups;

    enumerate(games, [&](int i, auto& game) {
        std::string key = (game.appid != "N/A") ? game.appid : cache_key(game);

        auto it = key_to_group.find(key);
        if (it != key_to_group.end()) {
            groups[it->second].push_back(i);
        } else {
            key_to_group[key] = groups.size();
            groups.push_back({static_cast<int>(i)});
        }
    });

    return groups;
}
