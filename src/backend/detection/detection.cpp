#include "detection.hpp"
#include "backend/config/config.hpp"
#include "backend/utils/blacklist/blacklist.hpp"
#include "backend/utils/paths.hpp"
#include "backend/logger/logger.hpp"

#include "backend/detection/rsg/rsg.hpp"
#include "backend/detection/ubi/ubi.hpp"
#include "backend/detection/unreal/unreal.hpp"
#include "backend/detection/custom/custom.hpp"
#include "globals.hpp"

struct Detectors {
   RockstarDetector rockstar_detect; 
   UbisoftDetector ubisoft_detect; 
   UnrealDetector unreal_detect; 
   CustomDetector custom_detect; 
};

std::vector<std::string> get_platform_steam_paths() {
#ifdef __APPLE__
    return {
        paths::home_dir().string() / "Library" / "Application Support" / "Steam" / "steamapps" / "libraryfolders.vdf",
    };
#endif

    try {
#ifdef __linux__
        return {
            paths::home_dir().string() + "/.steam/steam/steamapps/libraryfolders.vdf",
            paths::home_dir().string() + "/.local/share/Steam/steamapps/libraryfolders.vdf"
        };
#endif

#ifdef _WIN32
        return {
            "C:\\Program Files (x86)\\Steam\\steamapps\\libraryfolders.vdf",
        };
#endif
    } catch (const std::exception& e) {
        get_logger().error("Failed to get Steam paths: {}", std::string(e.what()));
        return {};
    }
}

std::optional<fs::path> get_steam_location(const Config& config) {
    if (!config.settings.steam_path.empty()) {
        if (fs::exists(config.settings.steam_path)) {
            return config.settings.steam_path;
        }
        get_logger().warning("Configured Steam path does not exist, falling back to defaults");
    }

    for (const auto& entry : get_platform_steam_paths()) {
        if (fs::exists(entry)) {
            return entry;
        }
    }

    return std::nullopt;
}

std::vector<fs::path> get_library_folders(Config& config) {
    auto vdf_file = get_steam_location(config);
    std::vector<fs::path> libraries;

    if(!vdf_file) {
        get_logger().warning("Steam installation not found");
        return {};
    }

    if (config.settings.steam_path.empty()) {
        config.settings.steam_path = vdf_file.value().string();
    }

    std::ifstream file(vdf_file.value().string());
    std::string line;

    if(!file.is_open()) {
        get_logger().error("Failed to open Steam library file");
        return {};
    }

    while (std::getline(file, line)) {
        if(line.find("\"path\"") != std::string::npos) {
            size_t first_quote = line.find('"');
            size_t second_quote = line.find('"', first_quote + 1);
            size_t third_quote = line.find('"', second_quote + 1);
            size_t fourth_quote = line.find('"', third_quote + 1);

            if(fourth_quote == std::string::npos) {
                continue;
            }

            std::string path_value = line.substr(third_quote + 1, fourth_quote - third_quote - 1);
            libraries.push_back(path_value);
        }
    }

    file.close();
    return libraries;
}

void Detection::add_game(std::expected<std::vector<Game>, DetectionError> result, const std::string& platform, std::vector<Game>& games) {
    if (result) {
        games.append_range(result.value());
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
                Detection::add_game(detectors.ubisoft_detect.find_saves(drive_c / "Program Files (x86)" / "Ubisoft" / "Ubisoft Game Launcher" / "savegames"), "ubi", result.games);
            }
            Detection::add_game(detectors.custom_detect.find_saves(drive_c), "custom", result.games); //might be too broad

            if (fs::exists(users_dir)) {
                for (const auto& user : fs::directory_iterator(users_dir)) {
                    if (user.path().filename() == "Public") continue;

                    if(config.settings.ubi_enabled) {
                        Detection::add_game(detectors.ubisoft_detect.find_saves(user.path() / "Documents"), "ubi", result.games);
                    }
                    if (config.settings.rsg_enabled) {
                        Detection::add_game(detectors.rockstar_detect.find_saves(user.path() / "Documents" / "Rockstar Games"), "rsg", result.games);
                        Detection::add_game(detectors.rockstar_detect.find_legacy_saves(user.path() / "Documents"), "rsg", result.games);
                        Detection::add_game(detectors.rockstar_detect.find_legacy_saves(user.path() / "AppData" / "Local" / "Rockstar Games"), "rsg", result.games);
                    }
                    if (config.settings.unreal_enabled) {
                        Detection::add_game(detectors.unreal_detect.find_saves(user.path()), "unreal", result.games);
                    }
                }
            }
        }
    } catch(const std::filesystem::filesystem_error& fse) {
        get_logger().error("scan_prefix_dir: {}", fse.what());
    };
}

Detection::DetectionResult Detection::find_saves(Config& config) {
    Detection::DetectionResult result;
    Detectors detectors;

#ifdef __linux__
    // steam
    auto libraries = get_library_folders(config);
    for (const auto& library : libraries) {
        fs::path compatdata = library / "steamapps/compatdata";
        if (!fs::exists(compatdata)) continue;

        scan_prefix_dir(compatdata, result, config, detectors);
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

        scan_prefix_dir(resolved_lutris, result, config, detectors);
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

        scan_prefix_dir(heroic_dir, result, config, detectors);
    } else {
        get_logger().warning("Heroic path does not exist!");
    }
#endif

#ifdef _WIN32
    if(config.settings.ubi_enabled) {
        Detection::add_game(detectors.ubisoft_detect.find_saves("C:\\Program Files (x86)\\Ubisoft\\Ubisoft Game Launcher\\savegames"), "ubi", result.games);
        Detection::add_game(detectors.ubisoft_detect.find_anno_saves(paths::documents_dir()), "ubi", result.games);
        Detection::add_game(detectors.ubisoft_detect.find_anno_saves(paths::home_dir() / "AppData" / "Roaming"), "ubi", result.games);
    }

    if(config.settings.rsg_enabled) {
        Detection::add_game(detectors.rockstar_detect.find_saves(paths::documents_dir() / "Rockstar Games"), "rsg", result.games);
        Detection::add_game(detectors.rockstar_detect.find_legacy_saves(paths::documents_dir()), "rsg", result.games);
        Detection::add_game(detectors.rockstar_detect.find_legacy_saves(paths::home_dir() / "AppData" / "Local" / "Rockstar Games"), "rsg", result.games);
    }
    if (config.settings.unreal_enabled) {
        Detection::add_game(detectors.unreal_detect.find_saves(paths::home_dir()), "unreal", result.games);
    }
    Detection::add_game(detectors.custom_detect.find_saves(paths::home_dir()), "custom", result.games);
#endif

#ifdef __APPLE__
    if(config.settings.ubi_enabled) {
        //ignored
    }

    if(config.settings.rsg_enabled) {
        //ignored
    }
    if (config.settings.unreal_enabled) {
        Detection::add_game(detectors.unreal_detect.find_saves(paths::home_dir() / "Library" / "Application Support", UnrealDetector::ScanMode::Native), "unreal", result.games); 
        Detection::add_game(detectors.unreal_detect.find_saves(paths::heroic_dir() / "Prefixes"), "unreal", result.games); 
    }
    Detection::add_game(detectors.custom_detect.find_saves(paths::home_dir()), "custom", result.games); 
#endif

    if (result.games.empty()) {
        get_logger().error("No savegames found!");
    }

    std::erase_if(result.games, [](const Game& game) {
        return Blacklist::is_blacklisted(game.game_name);
    });
    std::erase_if(result.games, [](const Game& game) {
        return !fs::is_directory(game.save_path) || fs::is_empty(game.save_path);
    });
    return result;
}

std::vector<std::vector<int>> Detection::DetectionResult::get_grouped() const {
    std::unordered_map<std::string, size_t> key_to_group;
    std::vector<std::vector<int>> groups;

    for (auto [i, game] : std::views::enumerate(games)) {
        std::string key = (game.appid != "N/A") ? game.appid : "N/A::" + game.game_name;

        auto it = key_to_group.find(key);
        if (it != key_to_group.end()) {
            groups[it->second].push_back(i);
        } else {
            key_to_group[key] = groups.size();
            groups.push_back({static_cast<int>(i)});
        }
    }

    return groups;
}
