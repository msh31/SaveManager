//this sucks | update: yes

#include "tabs.hpp"
#include "core/features/features.hpp"
#include "core/helpers/paths.hpp"

#include "core/logger/logger.hpp"
#include "core/network/network.hpp"
#include "core/ui/notifications/notification.hpp"
#include "core/helpers/remote_transfer/remote_transfer.hpp"
#include "imgui.h"
#include <atomic>

bool open_restore_modal, open_delete_modal = false;
std::vector<fs::path> backups;
const Game* pending_restore_game = nullptr;
const Game* pending_delete_game = nullptr;
int selected_backup_idx = 0;
static double last_read_time = 0.0;

void Tabs::render_general_tab(const Fonts& fonts, const Detection::DetectionResult& result, std::unordered_map<std::string, GLuint> texture_id, Config& config) {
    ImGui::PushFont(fonts.header);
    ImGui::Text("Detected Games");
    ImGui::PopFont();

    if(ImGui::Button("Refresh")) {
        Detection::find_saves(config);
        Notify::show_notification("Save refresh", "Saves have been refreshed succesfully!", 2000);
    }

    std::vector<std::vector<const Game*>> grouped_games;
    std::unordered_map<std::string, size_t> appid_to_group;

    for (const auto& game : result.games) {
        if (game.appid != "N/A") {
            auto it = appid_to_group.find(game.appid);
            if (it != appid_to_group.end()) {
                grouped_games[it->second].push_back(&game);
            } else {
                appid_to_group[game.appid] = grouped_games.size();
                grouped_games.push_back({&game});
            }
        } else {
            grouped_games.push_back({&game});
        }
    }

    static std::unordered_map<std::string, int> selected_source;

    int count = 0;
    float available_width = ImGui::GetContentRegionAvail().x;
    constexpr float card_width = 300.0f;
    constexpr float card_height = 300.0f;
    constexpr float card_gap = 10.0f;
    constexpr float image_height = 131.0f;
    int columns = (std::max)(1, (int)(available_width / (card_width + card_gap)));
    if(!grouped_games.empty()) {
        for (int gi = 0; gi < (int)grouped_games.size(); gi++) {
            const auto& group = grouped_games[gi];
            if(count > 0 && count % columns == 0) {
                ImGui::Dummy(ImVec2(0.0f, card_gap));
            }

            if(count % columns != 0) {
                ImGui::SameLine(0.0f, card_gap);
            }

            count++;
            const Game* primary = group[0];
            std::string card_id = primary->game_name + "##card" + std::to_string(gi);

            int& sel = selected_source[card_id];
            if (sel >= (int)group.size()) sel = 0;
            const Game& active_game = *group[sel];

            ImGui::BeginChild(card_id.c_str(), ImVec2(card_width, card_height), true);
            ImGui::TextWrapped("%s", primary->game_name.c_str());
            ImGui::Separator();

            ImGui::Dummy(ImVec2(0.0f, 8.0f));

            auto it = texture_id.find(primary->appid);
            if(it != texture_id.end()) {
                ImGui::Image((ImTextureID)(intptr_t)it->second, ImVec2(280, image_height));
            }
            else {
                ImGui::Dummy(ImVec2(280, image_height));
                ImVec2 pos = ImGui::GetCursorPos();
                ImGui::SetCursorPos(ImVec2(pos.x, pos.y - image_height / 2.0f - ImGui::GetTextLineHeight() / 2.0f));
                ImGui::TextDisabled("No image available");
                ImGui::SetCursorPos(pos);
            }

            if (group.size() > 1) {
                ImGui::SetNextItemWidth(-1);
                if (ImGui::BeginCombo("##source", active_game.save_path.string().c_str())) {
                    for (int i = 0; i < (int)group.size(); i++) {
                        bool is_selected = (sel == i);
                        std::string label = group[i]->save_path.string() + "##" + std::to_string(i);
                        if (ImGui::Selectable(label.c_str(), is_selected)) {
                            sel = i;
                        }
                        if (is_selected) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
            } else {
                ImGui::Dummy(ImVec2(0.0f, 8.0f));
            }

            if(ImGui::Button("Backup")) {
                Features::backup_game(active_game, config);
            }
            ImGui::SameLine();
            if(ImGui::Button("Restore")) {
                pending_restore_game = &active_game;
                open_restore_modal = true;
            }
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
            if(ImGui::Button("Delete")) {
                pending_delete_game = &active_game;
                open_delete_modal = true;
            }
            ImGui::PopStyleColor(2);

            ImGui::EndChild();
        }

        if(open_restore_modal) {
            open_restore_modal = false;
            backups = Features::get_backups(*pending_restore_game, config); 
            if(backups.empty()) {
                open_restore_modal = false;
            }
            ImGui::OpenPopup("Restore Backup");
        }
        if(open_delete_modal) {
            open_delete_modal = false;
            backups = Features::get_backups(*pending_delete_game, config); 
            if(backups.empty()) {
                open_delete_modal = false;
            }
            ImGui::OpenPopup("Delete Backup");
        }

        if(ImGui::BeginPopupModal("Restore Backup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::SetNextItemWidth(550.0f);
            if(ImGui::BeginListBox("##backups")) {
                for(int i = 0; i < backups.size(); i++) {
                    if(ImGui::Selectable(backups[i].filename().string().c_str(), selected_backup_idx == i)) {
                        selected_backup_idx = i;
                    }
                }
                ImGui::EndListBox();
            }

            if(ImGui::Button("Restore") && !backups.empty()) {
                Features::restore_backup(backups[selected_backup_idx], *pending_restore_game);
                ImGui::CloseCurrentPopup();
                open_restore_modal = false;
            }
            ImGui::SameLine();
            if(ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
                pending_restore_game = nullptr;
                open_restore_modal = false;
            }
            ImGui::EndPopup();
        }
        if(ImGui::BeginPopupModal("Delete Backup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::SetNextItemWidth(550.0f);
            if(ImGui::BeginListBox("##backups")) {
                for(int i = 0; i < backups.size(); i++) {
                    if(ImGui::Selectable(backups[i].filename().string().c_str(), selected_backup_idx == i)) {
                        selected_backup_idx = i;
                    }
                }
                ImGui::EndListBox();
            }

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
            if(ImGui::Button("Delete") && !backups.empty()) {
                if(fs::remove(backups[selected_backup_idx])) {
                    Notify::show_notification("Backup Deletion", "Backup deleted!", 1500);
                } else {
                    Notify::show_notification("Backup Deletion", "Backup could not be deleted!", 1500);
                }
                ImGui::CloseCurrentPopup();
                open_delete_modal = false;
            }
            ImGui::PopStyleColor(2);
            ImGui::SameLine();
            if(ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
                pending_delete_game = nullptr;
                open_delete_modal = false;
            }
            ImGui::EndPopup();
        }
    } else {
        ImGui::Text("None of the supported games were found on your system!");
    }
}

void Tabs::render_log_tab(const Fonts& fonts) {
    static std::string log_buffer;
    std::ifstream log_file(paths::config_dir() / "savemanager.log");

    if (ImGui::Button("Clear")) {
        std::ofstream clear_file(paths::config_dir() / "savemanager.log", std::ios::trunc);
        clear_file.close();
        log_buffer.clear();
    }

    if (ImGui::GetTime() - last_read_time > 2.0) {
        last_read_time = ImGui::GetTime();

        if (log_file.is_open()) {
            std::string buffer;
            log_buffer.clear();
            while (std::getline(log_file, buffer)) {
                log_buffer += buffer + "\n";
            }
        }
        else {
            ImGui::Text("the log could not be opened.");
        }
    }

    ImGui::TextUnformatted(log_buffer.c_str());
    // ImGui::SetScrollHereY(1.0f);
}

void Tabs::render_about_tab(const Fonts& fonts) {
    ImGui::NewLine();

    float win_width = ImGui::GetWindowSize().x;

    ImGui::PushFont(fonts.title);
    float title_width = ImGui::CalcTextSize("SaveManager").x;
    ImGui::SetCursorPosX((win_width - title_width) * 0.5f);
    ImGui::Text("SaveManager");
    ImGui::PopFont();

    ImGui::PushFont(fonts.medium);
    const char* subtitle = "The swiss army knife of save management.";
    float subtitle_width = ImGui::CalcTextSize(subtitle).x;
    ImGui::SetCursorPosX((win_width - subtitle_width) * 0.5f);
    ImGui::TextDisabled("%s", subtitle);
    ImGui::PopFont();

    ImGui::Dummy(ImVec2(0.0f, 4.0f));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0f, 4.0f));

    ImGui::PushFont(fonts.header);
    ImGui::Text("Details");
    ImGui::PopFont();

    ImGui::Text("Version");    ImGui::SameLine(120.0f); ImGui::Text("1.2.0");
    ImGui::Text("Author");     ImGui::SameLine(120.0f); ImGui::Text("marco007");
    ImGui::Text("License");    ImGui::SameLine(120.0f); ImGui::Text("GPLv3");
    ImGui::Text("Source");     ImGui::SameLine(120.0f);
    ImGui::TextLinkOpenURL("click for sauce", "https://github.com/msh31/SaveManager");

    ImGui::Dummy(ImVec2(0.0f, 4.0f));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0f, 4.0f));

    ImGui::PushFont(fonts.header);
    ImGui::Text("Description");
    ImGui::PopFont();

    ImGui::TextWrapped(
        "A tool for backing up and restoring game saves locally and over sftp. "
        "Supports Steam, Lutris, Unreal, Heroic, Ubisoft, Rockstar, and more."
    );

    ImGui::Dummy(ImVec2(0.0f, 4.0f));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0f, 4.0f));

    ImGui::PushFont(fonts.header);
    ImGui::Text("Built With");
    ImGui::PopFont();

    ImGui::Text("Dear ImGui | ");
    ImGui::SameLine();
    ImGui::Text("GLFW | ");
    ImGui::SameLine();
    ImGui::Text("OpenGL | ");
    ImGui::SameLine();
    ImGui::Text("libcurl | ");
    ImGui::SameLine();
    ImGui::Text("libzip | ");
    ImGui::SameLine();
    ImGui::Text("libssh2 ");
}

void Tabs::render_settings_tab(const Fonts& fonts, Config& config) {
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

    //needs improvement
    static bool initialized = false;
    static char backup_buf[256];
    static char lutris_buf[256];
    static char steam_buf[256];
    static char heroic_buf[256];

    if (!initialized) {
        snprintf(backup_buf, sizeof(backup_buf), "%s", config.settings.backup_path.string().c_str());
        snprintf(lutris_buf, sizeof(lutris_buf), "%s", config.settings.lutris_path.c_str());
        snprintf(steam_buf, sizeof(steam_buf), "%s", config.settings.steam_path.c_str());
        snprintf(heroic_buf, sizeof(heroic_buf), "%s", config.settings.heroic_path.c_str());
        initialized = true;
    }

    ImGui::InputText("Backup path", backup_buf, sizeof(backup_buf));
    ImGui::InputText("Lutris path", lutris_buf, sizeof(lutris_buf));
    ImGui::InputText("Steam path", steam_buf, sizeof(steam_buf));
    ImGui::InputText("Heroic path", heroic_buf, sizeof(heroic_buf));

    ImGui::Separator();

    if (ImGui::Button("Save")) {
        config.settings.backup_path = fs::path(backup_buf);
        config.settings.lutris_path = lutris_buf;
        config.settings.steam_path = steam_buf;
        config.settings.heroic_path = heroic_buf;
        config.save();
        Notify::show_notification("Config Saved!", "Settings saved successfully!", 1500);
    }
    ImGui::SameLine();
    if(ImGui::Button("Update translations")) {
        if(!Network::download_file("https://raw.githubusercontent.com/msh31/smdata/refs/heads/main/ubi_translations.json", paths::ubi_translations().string())) {
            get_logger().error("Failed to download Ubisoft translations");
        }
        if(!Network::download_file("https://raw.githubusercontent.com/msh31/smdata/refs/heads/main/steamids.json", paths::steam_appids().string())) {
            get_logger().error("Failed to download Steam ID data");
        }
        Notify::show_notification("Translations", "All translations have been updated!", 2500);
    }
}

void Tabs::render_transfer_tab(const Fonts& fonts, const Detection::DetectionResult& result, Config& config) {
    ImGui::PushFont(fonts.header);
    ImGui::Text("Save Transfer");
    ImGui::PopFont();
    ImGui::Separator();

    static bool initialized = false;
    static char dest_addr_buf[256];
    static char username_buf[256];
    static char password_buf[256];
    static char pubkey_buf[256];
    static char privkey_buf[256];
    static char remote_path_buf[256];
    static int selected_game_idx = 0;
    static std::vector<fs::path> backups;
    static std::vector<bool> selected_backups;

    if (!initialized) {
        snprintf(dest_addr_buf, sizeof(dest_addr_buf), "%s", config.sftp.dest_addr.c_str());
        snprintf(username_buf, sizeof(username_buf), "%s", config.sftp.username.c_str());
        snprintf(password_buf, sizeof(password_buf), "%s", config.sftp.password.c_str());
        snprintf(pubkey_buf, sizeof(pubkey_buf), "%s", config.sftp.pubkey.c_str());
        snprintf(privkey_buf, sizeof(privkey_buf), "%s", config.sftp.privkey.c_str());
        snprintf(remote_path_buf, sizeof(remote_path_buf), "%s", config.sftp.remote_path.c_str());
        initialized = true;
    }

    float window_width = ImGui::GetWindowSize().x;

    ImGui::BeginChild("##transfer_left", ImVec2(window_width * 0.5f - 20.0f, 0), false);

    ImGui::PushFont(fonts.medium);
    ImGui::Text("Server");
    ImGui::PopFont();
    ImGui::SetNextItemWidth(300.0f);
    ImGui::InputText("Address", dest_addr_buf, sizeof(dest_addr_buf));
    ImGui::SetNextItemWidth(300.0f);
    ImGui::InputText("Remote path", remote_path_buf, sizeof(remote_path_buf));

    ImGui::Dummy(ImVec2(0.0f, 10.0f));

    ImGui::PushFont(fonts.medium);
    ImGui::Text("Authentication");
    ImGui::PopFont();
    ImGui::SetNextItemWidth(145.0f);
    ImGui::InputText("Username##user", username_buf, sizeof(username_buf));
    ImGui::SameLine();
    ImGui::SetNextItemWidth(145.0f);
    ImGui::InputText("Password##pass", password_buf, sizeof(password_buf), ImGuiInputTextFlags_Password);

    ImGui::Dummy(ImVec2(0.0f, 5.0f));
    ImGui::TextDisabled("Or use SSH keys:");
    ImGui::SetNextItemWidth(300.0f);
    ImGui::InputText("Public key", pubkey_buf, sizeof(pubkey_buf));
    ImGui::SetNextItemWidth(300.0f);
    ImGui::InputText("Private key", privkey_buf, sizeof(privkey_buf));

    ImGui::Dummy(ImVec2(0.0f, 15.0f));

    static std::unique_ptr<RemoteTransfer> remote;
    static std::future<void> future;
    static std::atomic<int> current_file_index = 0;
    static std::atomic<int> total_files = 0;
    bool is_transferring = future.valid() && future.wait_for(std::chrono::seconds(0)) != std::future_status::ready;

    if(is_transferring) {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
    }

    if(ImGui::Button("Transfer")) {
        std::vector<fs::path> selected_paths;
        for (size_t i = 0; i < backups.size(); i++) {
            if (selected_backups[i]) {
                selected_paths.push_back(backups[i]);
            }
        }
        if (!selected_paths.empty()) {
            total_files = selected_paths.size();
            current_file_index = 0;
            remote = std::make_unique<RemoteTransfer>(config.sftp.dest_addr, selected_paths[0], config);

            future = std::async(std::launch::async, [r = remote.get(), selected_paths, &config]() {
                for (size_t i = 0; i < selected_paths.size(); i++) {
                    current_file_index = i;
                    r->transfer_file(selected_paths[i], config);
                }
            });
        } else {
            Notify::show_notification("Transfer", "No backups selected!", 2000);
        }
    }
    if(is_transferring) {
        ImGui::PopItemFlag();
        ImGui::PopStyleVar();
    }

    ImGui::SameLine();
    if (ImGui::Button("Save configuration")) {
        config.sftp.dest_addr = fs::path(dest_addr_buf).string();
        config.sftp.username = username_buf;
        config.sftp.password = password_buf;
        config.sftp.pubkey = fs::path(pubkey_buf);
        config.sftp.privkey = fs::path(privkey_buf);
        config.sftp.remote_path = remote_path_buf;
        config.save();
        Notify::show_notification("Config Saved!", "Settings saved successfully!", 1500);
    }

    ImGui::Dummy(ImVec2(0.0f, 10.0f));

    float file_progress = 0.0f;
    float overall_progress = 0.0f;
    bool transferring = is_transferring && remote;

    static bool was_transferring = false;

    if (transferring) {
        if (remote->total_bytes > 0) {
            file_progress = (float)remote->bytes_transferred / (float)remote->total_bytes;
        }
        if (total_files > 0) {
            overall_progress = ((float)current_file_index + file_progress) / (float)total_files;
        }
    } else if (was_transferring && !transferring) {
        Notify::show_notification("Transfer Complete", "All files transferred successfully!", 2000);
    }

    was_transferring = transferring;

    ImGui::Text("Current file");
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
    ImGui::ProgressBar(file_progress, ImVec2(-FLT_MIN, 0.0f), transferring ? std::to_string((int)(file_progress * 100)).c_str() : "Idle");
    ImGui::PopStyleColor();

    ImGui::Text("Overall");
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
    ImGui::ProgressBar(overall_progress, ImVec2(-FLT_MIN, 0.0f), transferring ? std::to_string((int)(overall_progress * 100)).c_str() : "Idle");
    ImGui::PopStyleColor();

    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("##transfer_right", ImVec2(0, 0), false);

    ImGui::PushFont(fonts.medium);
    ImGui::Text("Select Backups");
    ImGui::PopFont();

    std::vector<std::string> game_names;
    for (const auto& game : result.games) {
        game_names.push_back(game.game_name);
    }

    if (!game_names.empty()) {
        if (selected_game_idx >= (int)game_names.size()) selected_game_idx = 0;

        ImGui::SetNextItemWidth(300.0f);
        if (ImGui::BeginCombo("##game", game_names[selected_game_idx].c_str())) {
            for (int i = 0; i < (int)game_names.size(); i++) {
                bool is_selected = (selected_game_idx == i);
                if (ImGui::Selectable(game_names[i].c_str(), is_selected)) {
                    selected_game_idx = i;
                    backups = Features::get_backups(result.games[i], config);
                    selected_backups.clear();
                    selected_backups.resize(backups.size(), false);
                }
                if (is_selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        if (!backups.empty()) {
            if (ImGui::BeginListBox("##backups", ImVec2(-FLT_MIN, 200.0f))) {
                for (int i = 0; i < (int)backups.size(); i++) {
                    std::string label = backups[i].filename().string() + "##" + std::to_string(i);
                    if (ImGui::Selectable(label.c_str(), selected_backups[i], ImGuiSelectableFlags_AllowDoubleClick)) {
                        selected_backups[i] = !selected_backups[i];
                    }
                }
                ImGui::EndListBox();
            }

            int selected_count = 0;
            for (bool b : selected_backups) if (b) selected_count++;
            ImGui::Text("Selected: %d", selected_count);
        } else {
            ImGui::TextDisabled("No backups found");
        }
    } else {
        ImGui::TextDisabled("No games detected");
    }

    ImGui::EndChild();
}

void Tabs::render_debug_tab(const Fonts& fonts) {
    ImGui::PushFont(fonts.header);
    ImGui::Text("Debug menu");
    ImGui::PopFont();

    if(ImGui::Button("test notification")) {
        Notify::show_notification("test", "this is a test!", 2000);
        Notify::show_notification("test 2", "this is longer test to see how long text gets handled because I have some issues...", 3000);
    }
}
