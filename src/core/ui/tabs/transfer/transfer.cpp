#include "transfer.hpp"
#include "core/ui/notifications/notification.hpp"
#include "core/features/features.hpp"
#include "core/config/config.hpp"

void TransferTab::render(const Fonts& fonts, const Detection::DetectionResult& result, Config& config, TabState& state) {
    spinner_frame++;
    if (!initialized) {
        remote = std::make_unique<RemoteTransfer>();

        dest_addr = config.sftp.dest_addr;
        username = config.sftp.username;
        password = config.sftp.password;
        pubkey = config.sftp.pubkey.string();
        privkey = config.sftp.privkey.string();
        key_passphrase = config.sftp.key_passphrase;
        initialized = true;
    }

    bool is_transferring = future.valid() && future.wait_for(std::chrono::seconds(0)) != std::future_status::ready;
    bool is_connecting = connect_future.valid() && connect_future.wait_for(std::chrono::seconds(0)) != std::future_status::ready;

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

    // connection status updates that need to run every frame
    if (connect_future.valid() && connect_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
        bool success = connect_future.get();
        if (success) {
            connected = true;
            current_remote_path = "/home/" + config.sftp.username;
            remote_entries = remote->list_directory(current_remote_path);
            Notify::show_notification("SFTP Connection", "Connected!", 2000);
        } else {
            Notify::show_notification("SFTP Connection", "Failed to connect!", 2000);
        }
    }

    ImGui::PushFont(fonts.header);
    ImGui::Text("Save Transfer");
    ImGui::PopFont();
    ImGui::Separator();

    ImGui::BeginChild("##transfer_wrapper", ImVec2(0, ImGui::GetContentRegionAvail().y), false,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    float window_width = ImGui::GetWindowSize().x;
    float top_height = use_password_auth ? 290.0f : 370.0f;
    float half = (window_width - 20.0f) / 2.0f;

    ImGui::BeginChild("##server", ImVec2(half, top_height), true);
    ImGui::PushFont(fonts.medium);
    ImGui::Text("Server");
    ImGui::PopFont();

    ImGui::SetNextItemWidth(250.0f);
    ImGui::InputText("Address", &dest_addr);

    ImGui::Dummy(ImVec2(0.0f, 5.0f));

    ImGui::PushFont(fonts.medium);
    ImGui::Text("Authentication");
    ImGui::PopFont();

    if(ImGui::RadioButton("Password", use_password_auth)) {
        use_password_auth = true;
    }
    ImGui::SameLine();
    if(ImGui::RadioButton("SSH Key", !use_password_auth)) {
        use_password_auth = false;
    }

    ImGui::SetNextItemWidth(120.0f);
    ImGui::InputText("Username##user", &username);

    if(use_password_auth) {
        ImGui::SetNextItemWidth(120.0f);
        ImGui::InputText("Password##user", &password, ImGuiInputTextFlags_Password);
    } else {
        ImGui::SetNextItemWidth(250.0f);
        ImGui::InputText("Public key", &pubkey);
        ImGui::SetItemTooltip("Path to your public ssh key in full");
        ImGui::SetNextItemWidth(250.0f);
        ImGui::InputText("Private key", &privkey);
        ImGui::SetItemTooltip("Path to your private ssh key in full");
        ImGui::SetNextItemWidth(250.0f);
        ImGui::InputText("Key passphrase (Optional)", &key_passphrase, ImGuiInputTextFlags_Password);
        ImGui::SetItemTooltip("Your passphrase for the ssh key");
    }

    // ImGui::Dummy(ImVec2(0.0f, 10.0f));

    float status_y = top_height - 45.0f;
    ImGui::SetCursorPosY(status_y);
    if(!is_connecting && !connected) {
        if (ImGui::Button("Connect")) {
            connect_future = std::async(std::launch::async, [this, r = remote.get(), &config, auth = use_password_auth, pass = key_passphrase]() -> bool {
                return r->connect(dest_addr, config, auth, pass);
            });
        }
    } else if(!connected) {
        char spin_char = spinner[(spinner_frame / 10) % 4];

        std::string loading_text = std::string("Connecting... ") + spin_char;
        ImGui::Text("%s", loading_text.c_str());
    }
    if(connected) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
        if(ImGui::Button("Disconnect")) {
            connected = false;
            remote_entries = {};
            remote->disconnect();
            Notify::show_notification("SFTP Connection", "Disconnected from server!", 2000);
        }
        ImGui::PopStyleColor(2);
    }
    ImGui::SameLine();
    if (connected) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 1.0f, 0.3f, 1.0f));
        ImGui::BulletText("Connected to %s", dest_addr.c_str());
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        ImGui::BulletText("Not connected");
        ImGui::PopStyleColor();
    }
    ImGui::EndChild();

    ImGui::SameLine(0.0f, 10.0f);

    ImGui::BeginChild("##progress", ImVec2(half, top_height), true);
    ImGui::Text("File:");
    ImGui::SameLine(140.0f);
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
    ImGui::ProgressBar(file_progress, ImVec2(300.0f, 0.0f), transferring ? std::to_string((int)(file_progress * 100)).c_str() : "Idle");
    ImGui::PopStyleColor();

    ImGui::Text("Overall:");
    ImGui::SameLine(140.0f);
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
    ImGui::ProgressBar(overall_progress, ImVec2(300.0f, 0.0f), transferring ? std::to_string((int)(overall_progress * 100)).c_str() : "Idle");
    ImGui::PopStyleColor();

    ImGui::SetCursorPosY(status_y);
    if (ImGui::Button("Save configuration")) {
        config.sftp.dest_addr = fs::path(dest_addr).string();
        config.sftp.username = username;
        config.sftp.password = password;
        config.sftp.pubkey = fs::path(pubkey);
        config.sftp.privkey = fs::path(privkey);
        config.sftp.key_passphrase = key_passphrase;
        config.sftp.auth_pw = use_password_auth;
        config.save();
        Notify::show_notification("Config Saved!", "Settings saved successfully!", 1500);
    }
    ImGui::EndChild();
    ImGui::Dummy(ImVec2(0, 8.0f));

    float bottom_height = ImGui::GetContentRegionAvail().y;

    ImGui::BeginChild("##transfer_local", ImVec2(half, bottom_height), true);
    ImGui::PushFont(fonts.medium);
    ImGui::Text("Local");
    ImGui::PopFont();

    int local_selected_count = 0;
    for (bool b : state.selected_backups) if (b) local_selected_count++;

    ImGui::BeginDisabled(!connected || local_selected_count == 0 || is_transferring);
    if (ImGui::Button("Upload")) {
        std::vector<fs::path> selected_paths;
        for (size_t i = 0; i < state.backups.size(); i++) {
            if (state.selected_backups[i]) {
                selected_paths.push_back(state.backups[i]);
            }
        }
        if (!selected_paths.empty()) {
            total_files = selected_paths.size();
            current_file_index = 0;

            future = std::async(std::launch::async, [this, r = remote.get(), selected_paths, &config]() {
                for (size_t i = 0; i < selected_paths.size(); i++) {
                    current_file_index = i;
                    r->upload_file(selected_paths[i], current_remote_path, config);
                }
            });
        }
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    ImGui::Text("(%d selected)", local_selected_count);

    float content_height = ImGui::GetContentRegionAvail().y - 10.0f;

    auto groups = result.get_grouped();
    std::vector<std::string> game_names;
    for (const auto& group : groups) {
        game_names.push_back(result.games[group[0]].game_name);
    }

    if (!game_names.empty()) {
        if (state.selected_game_idx >= (int)game_names.size()) state.selected_game_idx = 0;

        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::BeginCombo("##game", game_names[state.selected_game_idx].c_str())) {
            for (int i = 0; i < (int)game_names.size(); i++) {
                bool is_selected = (state.selected_game_idx == i);
                if (ImGui::Selectable(game_names[i].c_str(), is_selected)) {
                    state.selected_game_idx = i;
                    state.backups = Features::get_backups(result.games[i], config);
                    state.selected_backups.clear();
                    state.selected_backups.resize(state.backups.size(), false);
                }
                if (is_selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        if (!state.backups.empty()) {
            if (ImGui::BeginListBox("##backups", ImVec2(-FLT_MIN, content_height))) {
                for (int i = 0; i < (int)state.backups.size(); i++) {
                    std::string label = state.backups[i].filename().string() + "##" + std::to_string(i);
                    if (ImGui::Selectable(label.c_str(), state.selected_backups[i], ImGuiSelectableFlags_AllowDoubleClick)) {
                        state.selected_backups[i] = !state.selected_backups[i];
                    }
                }
                ImGui::EndListBox();
            }
        } else {
            ImGui::TextDisabled("No backups found");
        }
    } else {
        ImGui::TextDisabled("No games detected");
    }
    ImGui::EndChild();

    ImGui::SameLine(0.0f, 10.0f);

    ImGui::BeginChild("##transfer_remote", ImVec2(half, bottom_height), true);
    ImGui::PushFont(fonts.medium);
    ImGui::Text("Remote");
    ImGui::PopFont();

    if (!connected) {
        ImGui::TextDisabled("Connect to browse remote server");
    } else {
        bool has_remote_selection = selected_remote_idx >= 0 && selected_remote_idx < (int)remote_entries.size();
        bool is_file_selected = has_remote_selection && !remote_entries[selected_remote_idx].is_directory;

        ImGui::BeginDisabled(!is_file_selected || is_transferring);
        if (ImGui::Button("Download")) {
            std::string path = current_remote_path + (current_remote_path.back() == '/' ? "" : "/") + remote_entries[selected_remote_idx].name;
            remote->download_file(path, config);
            Notify::show_notification("Remote Transfer", "Backup has been downloaded!", 2000);
        }
        ImGui::EndDisabled();

        ImGui::SameLine();
        if (has_remote_selection) {
            ImGui::Text("%s", remote_entries[selected_remote_idx].name.c_str());
        } else {
            ImGui::TextDisabled("(no selection)");
        }

        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        ImGui::Text("Path:");
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::Text("%s", current_remote_path.c_str());

        float remote_content_height = ImGui::GetContentRegionAvail().y - 10.0f;

        if (ImGui::BeginListBox("##remote_entries", ImVec2(-FLT_MIN, remote_content_height))) {
            if (current_remote_path != "/") {
                if (ImGui::Selectable("..##parent", false)) {
                    if (current_remote_path == "/") {
                        current_remote_path = "/";
                    } else {
                        current_remote_path = fs::path(current_remote_path).parent_path().string();
                        if (current_remote_path.empty()) current_remote_path = "/";
                    }
                    remote_entries = remote->list_directory(current_remote_path);
                    selected_remote_idx = -1;
                }
            }
            for (int i = 0; i < (int)remote_entries.size(); i++) {
                if (remote_entries[i].name == "." || remote_entries[i].name == "..") continue;

                std::string prefix = remote_entries[i].is_directory ? "[DIR] " : "[FILE] ";
                std::string label = prefix + remote_entries[i].name + "##" + std::to_string(i);
                if (ImGui::Selectable(label.c_str(), selected_remote_idx == i, ImGuiSelectableFlags_AllowDoubleClick)) {
                    if (ImGui::IsMouseDoubleClicked(0) && remote_entries[i].is_directory) {
                        current_remote_path = current_remote_path + (current_remote_path.back() == '/' ? "" : "/") + remote_entries[i].name;
                        remote_entries = remote->list_directory(current_remote_path);
                        selected_remote_idx = -1;
                    } else {
                        selected_remote_idx = i;
                    }
                }
            }
            ImGui::EndListBox();
        }
    }
    ImGui::EndChild();
    ImGui::EndChild();
}
