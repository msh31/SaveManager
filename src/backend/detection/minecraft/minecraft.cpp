#include "minecraft.hpp"
#include "backend/utils/paths.hpp"
#include "backend/logger/logger.hpp"
#include "types.hpp"

std::expected<std::vector<Game>, DetectionError> MinecraftDetector::find_saves() const {
    ZoneScopedN("minecraft_find_saves");
    std::vector<Game> games;

    auto append = [&](std::vector<Game> result) {
        games.insert(games.end(), result.begin(), result.end());
    };

    append(scan_official());
    append(scan_modrinth());
    append(scan_prism());
    append(scan_multimc());
    return games;
}

std::vector<Game> MinecraftDetector::scan_official() const {
    fs::path game_path = paths::home_dir() / ".minecraft";
    std::vector<Game> games;

    get_logger().debug("Scanning official Minecraft path: {}", game_path.string());

    if(!fs::exists(game_path)) {
        get_logger().debug("Official Minecraft path does not exist: {}", game_path.string());
        return {};
    }

    for(const auto& game : fs::directory_iterator(game_path, std::filesystem::directory_options::skip_permission_denied)) {
        fs::path saves_folder = game.path();
        if(saves_folder.filename().string() != "saves") continue;

        get_logger().debug("Found saves folder: {}", saves_folder.string());

        for(const auto& world : fs::directory_iterator(saves_folder, std::filesystem::directory_options::skip_permission_denied)) {
            if(world.path().empty()) continue;

            get_logger().debug("Found world save: {}", world.path().string());

            Game entry;
            entry.type = PlatformType::MINECRAFT;
            entry.game_name = "Minecraft";
            entry.appid = "N/A";
            entry.save_path = world.path().string();
            entry.launcher = LauncherType::OFFICIAL;
            games.push_back(entry);
        }
    }
    get_logger().debug("Official scan complete. Found {} saves", games.size());
    return games;
}

std::vector<Game> MinecraftDetector::scan_modrinth() const {
    fs::path modrinth_path = paths::home_dir() / ".local" / "share" / "ModrinthApp" / "profiles";
    std::vector<Game> games;

    get_logger().debug("Scanning Modrinth path: {}", modrinth_path.string());

    if(!fs::exists(modrinth_path)) {
        get_logger().debug("Modrinth path does not exist: {}", modrinth_path.string());
        return {};
    }

    for(const auto& game : fs::directory_iterator(modrinth_path, std::filesystem::directory_options::skip_permission_denied)) {
        fs::path game_folder = game.path();
        std::string folder_name = game_folder.filename().string();
        get_logger().debug("Checking Modrinth profile folder: {}", folder_name);

        for(const auto& profile : fs::directory_iterator(game_folder, std::filesystem::directory_options::skip_permission_denied)) {
            if(profile.path().filename().string() != "saves") continue;

            get_logger().debug("Found saves folder in Modrinth profile {}: {}", folder_name, profile.path().string());

            for(const auto& world : fs::directory_iterator(profile.path(), std::filesystem::directory_options::skip_permission_denied)) {
                if(world.path().empty()) continue;

                get_logger().debug("Found world save in Modrinth {}: {}", folder_name, world.path().string());

                Game entry;
                entry.type = PlatformType::MINECRAFT;
                entry.game_name = "Minecraft";
                entry.appid = "N/A";
                entry.save_path = world.path().string();
                entry.launcher = LauncherType::MODRINTH;
                games.push_back(entry);
            }
        }
    }
    get_logger().debug("Modrinth scan complete. Found {} saves", games.size());
    return games;
}

// std::vector<Game> MinecraftDetector::scan_curseforge() const {
//
// }
//
std::vector<Game> MinecraftDetector::scan_prism() const {
    fs::path prism_path = paths::home_dir() / ".var" / "app" / "org.prismlauncher.PrismLauncher" / "data" / "PrismLauncher" / "instances";
    std::vector<Game> games;

    get_logger().debug("Scanning Prism Launcher path: {}", prism_path.string());

    if(!fs::exists(prism_path)) {
        get_logger().debug("Prism Launcher path does not exist: {}", prism_path.string());
        return {};
    }

    for(const auto& game : fs::directory_iterator(prism_path, std::filesystem::directory_options::skip_permission_denied)) {
        fs::path game_folder = game.path();
        std::string folder_name = game_folder.filename().string();
        if(!fs::is_directory(game_folder)) continue;

        get_logger().debug("Checking Prism instance: {}", folder_name);

        for(const auto& folder : fs::directory_iterator(game_folder, std::filesystem::directory_options::skip_permission_denied)) {
            if(folder.path().filename().string() != "minecraft") continue;
            if(!fs::is_directory(folder)) continue;

            get_logger().debug("Found minecraft folder in Prism instance {}: {}", folder_name, folder.path().string());

            for(const auto& profile : fs::directory_iterator(folder, std::filesystem::directory_options::skip_permission_denied)) {
                if(profile.path().filename().string() != "saves") continue;
                if(!fs::is_directory(profile)) continue;

                get_logger().debug("Found saves folder in Prism instance {}: {}", folder_name, profile.path().string());

                for(const auto& world : fs::directory_iterator(profile.path(), std::filesystem::directory_options::skip_permission_denied)) {
                    if(world.path().empty()) continue;

                    get_logger().debug("Found world save in Prism instance {}: {}", folder_name, world.path().string());

                    Game entry;
                    entry.type = PlatformType::MINECRAFT;
                    entry.game_name = "Minecraft";
                    entry.appid = "N/A";
                    entry.save_path = world.path().string();
                    entry.launcher = LauncherType::PRISM;
                    games.push_back(entry);
                }
            }
        }
    }
    get_logger().debug("Prism Launcher scan complete. Found {} saves", games.size());
    return games;
}

std::vector<Game> MinecraftDetector::scan_multimc() const {
    fs::path multimc_path = paths::home_dir() / ".local" / "share" / "multimc" / "instances";
    std::vector<Game> games;

    get_logger().debug("Scanning MultiMC path: {}", multimc_path.string());

    if(!fs::exists(multimc_path)) {
        get_logger().debug("MultiMC path does not exist: {}", multimc_path.string());
        return {};
    }

    for(const auto& game : fs::directory_iterator(multimc_path, std::filesystem::directory_options::skip_permission_denied)) {
        fs::path game_folder = game.path();
        std::string folder_name = game_folder.filename().string();
        if(!fs::is_directory(game_folder)) continue;

        get_logger().debug("Checking MultiMC instance: {}", folder_name);

        for(const auto& profile : fs::directory_iterator(game_folder, std::filesystem::directory_options::skip_permission_denied)) {
            if(profile.path().filename().string() != "saves") continue;

            get_logger().debug("Found saves folder in MultiMC instance {}: {}", folder_name, profile.path().string());

            for(const auto& world : fs::directory_iterator(profile.path(), std::filesystem::directory_options::skip_permission_denied)) {
                if(world.path().empty()) continue;

                get_logger().debug("Found world save in MultiMC {}: {}", folder_name, world.path().string());

                Game entry;
                entry.type = PlatformType::MINECRAFT;
                entry.game_name = "Minecraft";
                entry.appid = "N/A";
                entry.save_path = world.path().string();
                entry.launcher = LauncherType::MULTIMC;
                games.push_back(entry);
            }
        }
    }
    get_logger().debug("MultiMC scan complete. Found {} saves", games.size());
    return games;
}
//
// std::vector<Game> MinecraftDetector::scan_atllauncher() const {
//
// }
