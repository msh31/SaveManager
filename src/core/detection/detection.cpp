#include <exception>
#include <filesystem>
#include <optional>

#include "detection.hpp"
#include "core/detection/ubi/ubi.hpp"
#include "core/detection/rsg/rsg.hpp"
#include "core/detection/unreal/unreal.hpp"
#include "core/helpers/paths.hpp"
#include "core/logger/logger.hpp"

std::vector<std::string> get_platform_steam_paths() {
#ifdef __APPLE__
    #warning "macOS not currently supported"
    return {};
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
        get_logger().error("Failed to get Steam paths: " + std::string(e.what()));
        return {};
    }
}

std::optional<fs::path> get_steam_location(Config& config) {
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

void Detection::scan_prefix_dir(const fs::path& compatdata, Detection::DetectionResult& result, const Config& config) {
    for (const auto& entry : fs::directory_iterator(compatdata)) {
        fs::path prefix = entry.path();
        if(!fs::exists(prefix)) {
            get_logger().warning("Prefix not found!");
            continue;
        }

        if (config.settings.ubi_enabled) {
            ubi::find_saves(prefix / "pfx/drive_c/Program Files (x86)/Ubisoft/Ubisoft Game Launcher/savegames", result.games, result.uuid);
        }
        fs::path users_dir = prefix / "pfx/drive_c/users/";
        if (fs::exists(users_dir)) {
            for (const auto& user : fs::directory_iterator(users_dir)) {
                if (user.path().filename() == "Public") continue;
                if (config.settings.rsg_enabled) {
                    rsg::find_saves(user.path() / "Documents/Rockstar Games", result.games);
                }
                if (config.settings.unreal_enabled) {
                    unreal::find_saves(user.path(), result.games);
                }
            }
        }
    }
}

Detection::DetectionResult Detection::find_saves(Config& config) {
    Detection::DetectionResult result;

#ifdef __linux__
    // steam
    auto libraries = get_library_folders(config);
    for (const auto& library : libraries) {
        fs::path compatdata = library / "steamapps/compatdata";
        if (!fs::exists(compatdata)) continue;

        scan_prefix_dir(compatdata, result, config);
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

        scan_prefix_dir(resolved_lutris, result, config);
    }
    //heroic
    fs::path heroic_dir = paths::heroic_dir() / "Prefixes/default";
    if (!config.settings.heroic_path.empty()) {
        if (fs::exists(config.settings.heroic_path)) {
            heroic_dir = config.settings.heroic_path;
        } else {
            get_logger().warning("Configured Heroic path does not exist, falling back to defaults");
        }
    }

    if (fs::exists(heroic_dir)) {
        if (config.settings.heroic_path.empty()) {
            config.settings.heroic_path = heroic_dir.string();
        }

        scan_prefix_dir(heroic_dir, result, config);
    } else {
        get_logger().warning("Heroic path does not exist!");
    }
#endif

#ifdef _WIN32
    if(config.settings.ubi_enabled) {
        ubi::find_saves("C:\\Program Files (x86)\\Ubisoft\\Ubisoft Game Launcher\\savegames", result.games, result.uuid);
    }

    if(config.settings.rsg_enabled) {
        rsg::find_saves(paths::documents_dir() / "Rockstar Games", result.games);
    }
#endif

    if (result.games.empty()) {
        get_logger().error("No savegames found!");
    }

    return result;
}
