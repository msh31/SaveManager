#include <filesystem>

#include "detection.hpp"
#include "core/detection/ubi/ubi.hpp"
#include "core/detection/rsg/rsg.hpp"
// #include "core/helpers/translations.hpp"
#include "core/logger/logger.hpp"

static logger detectLog;

std::vector<std::string> Detection::get_platform_steam_paths() {
#ifdef __linux__
    return {
        std::string(std::getenv("HOME")) + "/.steam/steam/steamapps/libraryfolders.vdf",
        std::string(std::getenv("HOME")) + "/.local/share/Steam/steamapps/libraryfolders.vdf"
    };
#endif

#ifdef _WIN32
    return {
        "C:\\Program Files (x86)\\Steam\\steamapps\\libraryfolders.vdf",
    };
#endif

#ifdef __APPLE__
    return {
        "macOS"
    };
#endif
}

std::optional<fs::path> Detection::get_steam_location() {
    for (auto entry : get_platform_steam_paths()) {
        if(fs::exists(entry)) {
            return entry;
        }
    }

    return std::nullopt;
}
//PUBLIC

std::vector<fs::path> Detection::get_library_folders() {
    auto vdf_file = get_steam_location();
    std::vector<fs::path> libraries;

    if(!vdf_file) {
        detectLog.warning("Steam installation not found");
        return {};
    }

    std::ifstream file(vdf_file.value().string());
    std::string line;

    if(!file.is_open()) {
        detectLog.error("Failed to open Steam library file");
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


Detection::DetectionResult Detection::find_saves() {
    DetectionResult result;

#ifdef __linux__
// steam
    auto libraries = get_library_folders();
    for (const auto& library : libraries) {
        fs::path compatdata = library / "steamapps/compatdata";
        if (!fs::exists(compatdata)) continue;

        for (const auto& entry : fs::directory_iterator(compatdata)) {
            fs::path prefix = entry.path();
            ubi::find_saves(prefix / "pfx/drive_c/Program Files (x86)/Ubisoft/Ubisoft Game Launcher/savegames", result.games, result.uuid);
            rsg::find_saves(prefix / "pfx/drive_c/users/steamuser/Documents/Rockstar Games", result.games);
        }
    }
// lutris
    fs::path lutris_root = fs::path(std::getenv("HOME")) / "Games";
    if (fs::exists(lutris_root)) {
        for (const auto& entry : fs::directory_iterator(lutris_root)) {
            fs::path prefix = entry.path();
            ubi::find_saves(prefix / "drive_c/Program Files (x86)/Ubisoft/Ubisoft Game Launcher/savegames", result.games, result.uuid);
            rsg::find_saves(prefix / "drive_c/users/steamuser/Documents/Rockstar Games", result.games); //untested
        }
    }
#endif

#ifdef _WIN32
    ubi::find_saves("C:\\Program Files (x86)\\Ubisoft\\Ubisoft Game Launcher\\savegames", result.games, result.uuid);
    rsg::find_saves(documents_dir / "Rockstar Games", result.games);
#endif

    if (result.games.empty()) {
        detectLog.error("No savegames found!");
    }

    return result;
}
