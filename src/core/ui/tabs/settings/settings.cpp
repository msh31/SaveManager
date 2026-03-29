#include "settings.hpp"
#include "core/helpers/blacklist/blacklist.hpp"
#include "core/helpers/custom_games/custom_games.hpp"
#include "core/ui/notifications/notification.hpp"
#include "core/network/network.hpp"
#include "core/config/config.hpp"
#include "core/logger/logger.hpp"

#include "imgui.h"
#include "imgui/misc/cpp/imgui_stdlib.h"

void SettingsTab::render(const Fonts& fonts, Config& config) {
    if (update_future.valid() && update_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
        bool result = update_future.get();

        if(result) {
            Notify::show_notification("Update Available!", "A new release is available for download!", 2500);
        } else {
            Notify::show_notification("Updates", "No new updates found!", 2500);
        }
    }

    bool is_checking = update_future.valid() && update_future.wait_for(std::chrono::seconds(0)) != std::future_status::ready;

    ImGui::PushFont(fonts.header);
    ImGui::Text("Settings");
    ImGui::PopFont();
   
    ImGui::PushFont(fonts.medium);
    ImGui::Text("Launcher Support");
    ImGui::PopFont();
    ImGui::Checkbox("Ubisoft Connect", &config.settings.ubi_enabled);
    ImGui::SameLine();
    ImGui::Checkbox("Rockstar Games Launcher", &config.settings.rsg_enabled);
    ImGui::SameLine();
    ImGui::Checkbox("Unreal Games (.sav saves)", &config.settings.unreal_enabled);
    ImGui::Separator();

    ImGui::PushFont(fonts.medium);
    ImGui::Text("Paths");
    ImGui::PopFont();

    if (!paths_initialized) {
        backup_path = config.settings.backup_path.string();
        steam_path = config.settings.steam_path;
        lutris_path = config.settings.lutris_path;
        heroic_path = config.settings.heroic_path;
        paths_initialized = true;
    }

    ImGui::InputText("Backup path", &backup_path);
    ImGui::InputText("Steam path", &steam_path);
    ImGui::InputText("Lutris path", &lutris_path);
    ImGui::InputText("Heroic path", &heroic_path);

    ImGui::Separator();

    if (ImGui::Button("Save")) {
        config.settings.backup_path = fs::path(backup_path).string();
        config.settings.steam_path = steam_path;
        config.settings.lutris_path = lutris_path;
        config.settings.heroic_path = heroic_path;
        config.save();
        Notify::show_notification("Config Saved!", "Settings saved successfully!", 1500);
    }

    ImGui::Separator();

    ImGui::BeginDisabled(is_checking);
    if(ImGui::Button("Check for updates")) {
            update_future = std::async(std::launch::async, []() {
                return Network::is_update_available();
            });
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    if(ImGui::Button("Update translations")) {
        if(!Network::download_file(ubi_translation_url, paths::ubi_translations().string())) {
            get_logger().error("Failed to download Ubisoft translations");
        }
        if(!Network::download_file(steam_translation_url, paths::steam_appids().string())) {
            get_logger().error("Failed to download Steam ID data");
        }
        Notify::show_notification("Translations", "All translations have been updated!", 2500);
    }

    ImGui::Separator();

    ImGui::PushFont(fonts.medium);
    ImGui::Text("Blacklisted Games");
    ImGui::PopFont();

    if (ImGui::BeginChild("blacklist_child", ImVec2(0, 120), true)) {
        for (auto it = Blacklist::blacklisted_games.begin(); it != Blacklist::blacklisted_games.end(); ) {
            ImGui::Text("%s", it->c_str());
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5);
            if (ImGui::Button(("X##" + std::to_string(std::distance(Blacklist::blacklisted_games.begin(), it))).c_str())) {
                it = Blacklist::blacklisted_games.erase(it);
                Blacklist::save();
            } else {
                ++it;
            }
        }
        ImGui::EndChild();
    }

    ImGui::InputText("##blacklist_input", &blacklist_input);
    ImGui::SameLine();
    if (ImGui::Button("Add##blacklist")) {
        if (!blacklist_input.empty()) {
            Blacklist::blacklisted_games.insert(blacklist_input);
            Blacklist::save();
            blacklist_input.clear();
        }
    }

    ImGui::PushFont(fonts.medium);
    ImGui::Text("Custom Games");
    ImGui::PopFont();

    if (ImGui::BeginChild("custom_game_child", ImVec2(0, 120), true)) {
        for (auto it = CustomGamesFile::games.begin(); it != CustomGamesFile::games.end(); ) {
            ImGui::Text("%s", it->game_name.c_str());
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5);
            if (ImGui::Button(("X##" + std::to_string(std::distance(CustomGamesFile::games.begin(), it))).c_str())) {
                it = CustomGamesFile::games.erase(it);
                CustomGamesFile::save();
            } else {
                ++it;
            }
        }
        ImGui::EndChild();
    }

    ImGui::InputText("Game Name", &new_game_name);
    ImGui::InputText("Save Path", &new_game_path);
    ImGui::InputText("AppID (optional)", &new_game_appid);
    // ImGui::SameLine();
    if (ImGui::Button("Add##custom")) {
        if (!new_game_name.empty() && !new_game_path.empty()) {
            CustomGamesFile::CustomGame game;
            game.game_name = new_game_name;
            game.save_path = new_game_path;
            game.appid = new_game_appid;
            CustomGamesFile::games.push_back(game);
            CustomGamesFile::save();

            new_game_name.clear();
            new_game_path.clear();
            new_game_appid.clear();
        }
    }
}
