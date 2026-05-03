#include "minecraft.hpp"
#include "backend/utils/paths.hpp"
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
    append(scan_curseforge());
    return games;
}

std::vector<Game> MinecraftDetector::scan_official() const {
    fs::path game_path = paths::home_dir() / ".minecraft";
    std::vector<Game> games;

    if(!fs::exists(game_path)) return {};

    for(const auto& game : fs::directory_iterator(game_path, std::filesystem::directory_options::skip_permission_denied)) {
        fs::path saves_folder = game.path();
        if(saves_folder.filename().string() != "saves") continue;

        for(const auto& world : fs::directory_iterator(saves_folder, std::filesystem::directory_options::skip_permission_denied)) {
            if(world.path().empty()) continue;

            Game entry;
            entry.type = PlatformType::MINECRAFT;
            entry.game_name = "Minecraft";
            entry.appid = "N/A";
            entry.save_path = world.path().string();
            entry.launcher = LauncherType::OFFICIAL;
            games.push_back(entry);
        }
    }
    return games;
}

std::vector<Game> MinecraftDetector::scan_modrinth() const {
    fs::path modrinth_path = paths::home_dir() / ".local" / "share" / "ModrinthApp" / "profiles";
    std::vector<Game> games;

    if(!fs::exists(modrinth_path)) return {};

    for(const auto& game : fs::directory_iterator(modrinth_path, std::filesystem::directory_options::skip_permission_denied)) {
        fs::path game_folder = game.path();
        std::string folder_name = game_folder.filename().string();

        for(const auto& profile : fs::directory_iterator(game_folder, std::filesystem::directory_options::skip_permission_denied)) {
            if(profile.path().filename().string() != "saves") continue;

            for(const auto& world : fs::directory_iterator(profile.path(), std::filesystem::directory_options::skip_permission_denied)) {
                if(world.path().empty()) continue;

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
    return games;
}

std::vector<Game> MinecraftDetector::scan_curseforge() const {
    fs::path curse_path = paths::home_dir() / "Documents" / "curseforge" / "minecraft" / "Instances";
    std::vector<Game> games;

    if(!fs::exists(curse_path)) return {};

    for(const auto& game : fs::directory_iterator(curse_path, std::filesystem::directory_options::skip_permission_denied)) {
        fs::path game_folder = game.path();
        std::string folder_name = game_folder.filename().string();

        for(const auto& profile : fs::directory_iterator(game_folder, std::filesystem::directory_options::skip_permission_denied)) {
            if(profile.path().filename().string() != "saves") continue;

            for(const auto& world : fs::directory_iterator(profile.path(), std::filesystem::directory_options::skip_permission_denied)) {
                if(world.path().empty()) continue;

                Game entry;
                entry.type = PlatformType::MINECRAFT;
                entry.game_name = "Minecraft";
                entry.appid = "N/A";
                entry.save_path = world.path().string();
                entry.launcher = LauncherType::CURSEFORGE;
                games.push_back(entry);
            }
        }
    }
    return games;
}

std::vector<Game> MinecraftDetector::scan_prism() const {
    fs::path prism_path = paths::home_dir() / ".var" / "app" / "org.prismlauncher.PrismLauncher" / "data" / "PrismLauncher" / "instances";
    std::vector<Game> games;

    if(!fs::exists(prism_path)) return {};

    for(const auto& game : fs::directory_iterator(prism_path, std::filesystem::directory_options::skip_permission_denied)) {
        fs::path game_folder = game.path();
        std::string folder_name = game_folder.filename().string();
        if(!fs::is_directory(game_folder)) continue;

        for(const auto& folder : fs::directory_iterator(game_folder, std::filesystem::directory_options::skip_permission_denied)) {
            if(folder.path().filename().string() != "minecraft") continue;
            if(!fs::is_directory(folder)) continue;

            for(const auto& profile : fs::directory_iterator(folder, std::filesystem::directory_options::skip_permission_denied)) {
                if(profile.path().filename().string() != "saves") continue;
                if(!fs::is_directory(profile)) continue;

                for(const auto& world : fs::directory_iterator(profile.path(), std::filesystem::directory_options::skip_permission_denied)) {
                    if(world.path().empty()) continue;

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
    return games;
}

std::vector<Game> MinecraftDetector::scan_multimc() const {
    fs::path multimc_path = paths::home_dir() / ".local" / "share" / "multimc" / "instances";
    std::vector<Game> games;

    if(!fs::exists(multimc_path)) return {};

    for(const auto& game : fs::directory_iterator(multimc_path, std::filesystem::directory_options::skip_permission_denied)) {
        fs::path game_folder = game.path();
        std::string folder_name = game_folder.filename().string();
        if(!fs::is_directory(game_folder)) continue;

        for(const auto& profile : fs::directory_iterator(game_folder, std::filesystem::directory_options::skip_permission_denied)) {
            if(profile.path().filename().string() != "saves") continue;

            for(const auto& world : fs::directory_iterator(profile.path(), std::filesystem::directory_options::skip_permission_denied)) {
                if(world.path().empty()) continue;

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
    return games;
}
//
// std::vector<Game> MinecraftDetector::scan_atllauncher() const {
//
// }
