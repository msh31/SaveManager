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
    // append(scan_prism());
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

// std::vector<Game> MinecraftDetector::scan_curseforge() const {
//
// }
//
// std::vector<Game> MinecraftDetector::scan_prism() const {
//
// }
//
// std::vector<Game> MinecraftDetector::scan_multimc() const {
//
// }
//
// std::vector<Game> MinecraftDetector::scan_atllauncher() const {
//
// }
