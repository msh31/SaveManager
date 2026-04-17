#include "settings.hpp"
#include "backend/utils/blacklist/blacklist.hpp"
#include "backend/utils/custom_games/custom_games.hpp"
#include "backend/utils/paths.hpp"
#include "frontend/ui/notifications/notification.hpp"
#include "backend/network/network.hpp"
#include "backend/config/config.hpp"
#include "backend/logger/logger.hpp"

void SettingsTab::render(const Fonts& fonts, Config& config) {
    spinner_frame++;

    if (update_future.valid() && update_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
        bool result = update_future.get();

        if(result) {
            Notify::show_notification("Update Available!", "A new release is available for download!", 2500);
        } else {
            Notify::show_notification("Updates", "No new updates found!", 2500);
        }
    }

    if (update_t_future.valid() && update_t_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
        auto [ubi, steam] = update_t_future.get();

        if(!ubi) {
            get_logger().error("Failed to download Ubisoft translations");
            Notify::show_notification("Translations", "Failed to update translations for ubisoft", 2500);
        }
        if(!steam) {
            get_logger().error("Failed to download Steam ID data");
            Notify::show_notification("Translations", "Failed to update translations for steam appids", 2500);
        }
        if(steam && ubi) {
            Notify::show_notification("Translations", "All translations have been updated!", 2500);
        }
    }

    bool is_checking = update_future.valid() && update_future.wait_for(std::chrono::seconds(0)) != std::future_status::ready;
    bool is_checking_t = update_t_future.valid() && update_t_future.wait_for(std::chrono::seconds(0)) != std::future_status::ready;

    float half = (ImGui::GetWindowSize().x - 20.0f) / 2.0f;

    ImGui::PushFont(fonts.header);
    ImGui::Text("Settings");
    ImGui::PopFont();

    ImGui::BeginChild("##appearance_support", ImVec2(half, 275.0f), true,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImGui::PushFont(fonts.medium);
    ImGui::Text("Appearance & Launchers");
    ImGui::PopFont();

    ImGui::Dummy(ImVec2(0.0f, 4.0f));
    ImGui::Checkbox("Dark Mode", &config.settings.dark_mode);
    ImGui::Separator();
   
    ImGui::PushFont(fonts.medium);
    ImGui::Text("Launcher Support");
    ImGui::PopFont();
    ImGui::Checkbox("Ubisoft Connect", &config.settings.ubi_enabled);
    ImGui::SameLine();
    ImGui::Checkbox("Rockstar Games Launcher", &config.settings.rsg_enabled);
    ImGui::SameLine();
    ImGui::Checkbox("Unreal Games (.sav saves)", &config.settings.unreal_enabled);

    ImGui::Separator();
    if(!is_checking) {
        if(ImGui::Button("Check for updates")) {
            update_future = std::async(std::launch::async, []() {
                return Network::is_update_available();
            });
        }
    } else {
        char spin_char = spinner[(spinner_frame / 10) % 4];
        ImGui::Text("%s", std::format("Checking for updates {}", spin_char).c_str());
    }
    ImGui::SameLine();
    if(!is_checking_t) {
        if(ImGui::Button("Update translations")) {
            update_t_future = std::async(std::launch::async, [this]() -> std::pair<bool, bool> {
                bool ubi = Network::download_file(ubi_translation_url, paths::ubi_translations().string());
                bool steam = Network::download_file(steam_translation_url, paths::steam_appids().string());
                return {ubi, steam};
            });
        }
        ImGui::SetItemTooltip("Forces a new download of the ubisoft id and steam id translations");
    } else {
        char spin_char = spinner[(spinner_frame / 10) % 4];
        ImGui::Text("%s", std::format("Updating translations {}", spin_char).c_str());
    }
    ImGui::SameLine();
    // if(ImGui::Button("Refresh Cache")) {
    //     std::error_code ec;
    //     fs::remove_all(paths::cache_dir(), ec);
    //     *m_refresh_requested = true;
    //
    //     if (ec) {
    //         get_logger().warning("cache refresh error: {}", ec.message());
    //     }
    // }
    ImGui::SetItemTooltip("Deletes cached images and re-downloads them.");
    ImGui::EndChild();

    ImGui::SameLine(0.0f, 10.0f);

    ImGui::BeginChild("##paths", ImVec2(half, 275.0f), true,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PushFont(fonts.medium);
    ImGui::Text("Paths");
    ImGui::PopFont();

    if (!paths_initialized) {
        backup_path = config.settings.backup_path.string();
        steam_path = config.settings.steam_path;
#ifdef __linux__
        lutris_path = config.settings.lutris_path;
#endif
        heroic_path = config.settings.heroic_path;
        paths_initialized = true;
    }

    ImGui::InputText("Backup path", &backup_path);
    ImGui::InputText("Steam path", &steam_path);
#ifdef __linux__
    ImGui::InputText("Lutris path", &lutris_path);
#endif
    ImGui::InputText("Heroic path", &heroic_path);

    ImGui::Separator();

    if (ImGui::Button("Save")) {
        config.settings.backup_path = fs::path(backup_path).string();
        config.settings.steam_path = steam_path;
#ifdef __linux__
        config.settings.lutris_path = lutris_path;
#endif
        config.settings.heroic_path = heroic_path;
        config.save();
        Notify::show_notification("Config Saved!", "Settings saved successfully!", 1500);
    }
    ImGui::EndChild();

    ImGui::BeginChild("##blacklisted_games", ImVec2(half, 310.0f), true,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImGui::PushFont(fonts.medium);
    ImGui::Text("Blacklisted Games");
    ImGui::PopFont();

    if (ImGui::BeginChild("blacklist_child", ImVec2(0, 120), true)) {
        int i = 0;
        for (auto it = Blacklist::blacklisted_games.begin(); it != Blacklist::blacklisted_games.end(); ) {
            ImGui::Text("%s", it->c_str());
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5);
            if (ImGui::Button(std::format("X##{}", i).c_str())) {
                it = Blacklist::blacklisted_games.erase(it);
                Blacklist::save();
            } else {
                ++it;
                ++i;
            }
        }
        ImGui::EndChild();
    } else {
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
    ImGui::EndChild();

    ImGui::SameLine(0.0f, 10.0f);

    ImGui::BeginChild("##custom_games", ImVec2(half, 310.0f), true,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImGui::PushFont(fonts.medium);
    ImGui::Text("Custom Games");
    ImGui::PopFont();

    if (ImGui::BeginChild("custom_game_child", ImVec2(0, 120), true)) {
        int i = 0;
        for (auto it = CustomGamesFile::games.begin(); it != CustomGamesFile::games.end(); ) {
            ImGui::Text("%s", it->game_name.c_str());
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5);
            if (ImGui::Button(std::format("X##{}", i).c_str())) {
                it = CustomGamesFile::games.erase(it);
                CustomGamesFile::save();
            } else {
                ++it;
                ++i;
            }
        }
        ImGui::EndChild();
    } else {
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
    ImGui::EndChild();
}
