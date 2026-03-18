#include "general.hpp"

void GeneralTab::render(const Fonts& fonts, const Detection::DetectionResult& result, std::unordered_map<std::string, GLuint> texture_id, Config& config) {
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
