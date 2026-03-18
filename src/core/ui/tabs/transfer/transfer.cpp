#include "transfer.hpp"
#include "core/helpers/remote_transfer/remote_transfer.hpp"
#include "core/ui/notifications/notification.hpp"
#include "core/features/features.hpp"
#include "core/config/config.hpp"

#include "imgui/misc/cpp/imgui_stdlib.h"

#include <future>

void TransferTab::render(const Fonts& fonts, const Detection::DetectionResult& result, Config& config) {
    ImGui::PushFont(fonts.header);
    ImGui::Text("Save Transfer");
    ImGui::PopFont();
    ImGui::Separator();

    static int selected_game_idx = 0;
    static std::vector<fs::path> backups;
    static std::vector<bool> selected_backups;

    static std::string dest_addr {config.sftp.dest_addr};
    static std::string username {config.sftp.username};
    static std::string password {config.sftp.password};
    static std::string pubkey {config.sftp.pubkey};
    static std::string privkey {config.sftp.privkey};
    static std::string remote_path {config.sftp.remote_path};

    float window_width = ImGui::GetWindowSize().x;

    ImGui::BeginChild("##transfer_left", ImVec2(window_width * 0.5f - 20.0f, 0), false);

    ImGui::PushFont(fonts.medium);
    ImGui::Text("Server");
    ImGui::PopFont();
    ImGui::SetNextItemWidth(300.0f);
    ImGui::InputText("Address", &dest_addr);
    ImGui::SetNextItemWidth(300.0f);
    ImGui::InputText("Remote path", &remote_path);

    ImGui::Dummy(ImVec2(0.0f, 10.0f));

    ImGui::PushFont(fonts.medium);
    ImGui::Text("Authentication");
    ImGui::PopFont();
    ImGui::SetNextItemWidth(145.0f);
    ImGui::InputText("Username##user", &username);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(145.0f);
    ImGui::InputText("Password##user", &password, ImGuiInputTextFlags_Password);

    ImGui::Dummy(ImVec2(0.0f, 5.0f));
    ImGui::TextDisabled("Or use SSH keys:");
    ImGui::SetNextItemWidth(300.0f);
    ImGui::InputText("Public key", &pubkey);
    ImGui::SetNextItemWidth(300.0f);
    ImGui::InputText("Private key", &privkey);

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
        config.sftp.dest_addr = fs::path(dest_addr).string();
        config.sftp.username = username;
        config.sftp.password = password;
        config.sftp.pubkey = fs::path(pubkey);
        config.sftp.privkey = fs::path(privkey);
        config.sftp.remote_path = remote_path;
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

