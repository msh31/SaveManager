#include "minecraft.hpp"
#include <filesystem>
#include <print>
// #include "backend/detection/detection.hpp"
// #include "backend/utils/translations/translations.hpp"

std::expected<std::vector<Game>, DetectionError> MinecraftDetector::find_saves(const fs::path& prefix) const {
    ZoneScopedN("minecraft_find_saves");
    if(!fs::exists(prefix)) {
        return std::unexpected{DetectionError::PathNotFound};
    }
    std::vector<Game> games;

    for(const auto& game : fs::directory_iterator(prefix, std::filesystem::directory_options::skip_permission_denied)) {
        fs::path game_folder = game.path();
        std::string folder_name = game_folder.filename().string();

        std::println("found modrinth profile: {}", folder_name);

        for(const auto& profile : fs::directory_iterator(game_folder, std::filesystem::directory_options::skip_permission_denied)) {
            if(profile.path().filename().string() != "saves") continue;
            // std::println("found profile: {}", profile.path().string());

            for(const auto& world : fs::directory_iterator(profile.path(), std::filesystem::directory_options::skip_permission_denied)) {
                std::println("found world: {}", world.path().string());
                if(world.path().empty()) continue;

                Game entry;
                entry.type = PlatformType::MINECRAFT;
                entry.game_name = "Minecraft";
                entry.appid = "N/A";
                entry.save_path = world.path().string();
                games.push_back(entry);
            }
        }
    }
    return games;
}
