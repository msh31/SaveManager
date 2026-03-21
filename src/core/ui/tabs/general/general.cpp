#include "general.hpp"
#include "core/globals.hpp"
#include "core/ui/notifications/notification.hpp"
#include "core/features/features.hpp"
#include <optional>

void GeneralTab::on_result_changed(Detection::DetectionResult& result, TabState& state) {
    grouped_games = {};
    appid_to_group = {};
    pending_restore_game = nullptr;
    pending_delete_game = nullptr;
    state.selected_game_idx = 0;
    state.selected_backup_idx = 0;

    for (int i = 0; i < (int)result.games.size(); i++) {
        const auto& game = result.games[i];
        if (game.appid != "N/A") {
            auto it = appid_to_group.find(game.appid);
            if (it != appid_to_group.end()) {
                grouped_games[it->second].push_back(i);
            } else {
                appid_to_group[game.appid] = grouped_games.size();
                grouped_games.push_back({i});
            }
        } else {
            grouped_games.push_back({i});
        }
    }

    last_game_count = result.games.size();
}

std::optional<Detection::DetectionResult> GeneralTab::render(const Fonts& fonts, Detection::DetectionResult& result, std::unordered_map<std::string, GLuint> texture_id, Config& config, TabState& state) {
    spinner_frame++;
    float available_width = ImGui::GetContentRegionAvail().x;
    float card_width = 300.0f;
    float card_height = 300.0f;
    float card_gap = 10.0f;
    float image_height = 131.0f;
    int columns = (std::max)(1, (int)(available_width / (card_width + card_gap)));
    int count = 0;

    if (refresh_future.valid() && refresh_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
        auto new_result = refresh_future.get();
        Notify::show_notification("Save refresh", "Saves refreshed!", 2000);
        return new_result;
    }

    if(last_game_count != result.games.size()) {
        on_result_changed(result, state);
    }

    ImGui::PushFont(fonts.header);
    ImGui::Text("Detected Games");
    ImGui::PopFont();

    bool is_refreshing = refresh_future.valid() && refresh_future.wait_for(std::chrono::seconds(0)) != std::future_status::ready;
    if(!is_refreshing) {
        if(ImGui::Button("Refresh")) {
            refresh_future = std::async(std::launch::async, [&config]() {
                return Detection::find_saves(config);
            });
        }
    } else {
        char spin_char = spinner[(spinner_frame / 10) % 4];

        std::string loading_text = std::string("Refreshing savegames... ") + spin_char;
        ImGui::Text("%s", loading_text.c_str());
    }

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
            const Game* primary = &result.games[group[0]];
            std::string card_id = primary->game_name + "##card" + std::to_string(gi);

            int& sel = selected_source[card_id];
            if (sel >= (int)group.size()) sel = 0;
            const Game& active_game = result.games[group[sel]];

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
                        std::string label = result.games[group[i]].save_path.string() + "##" + std::to_string(i);
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
            if(ImGui::Button("Open Path")) {
#ifdef __linux
                std::string cmd = "xdg-open \"" + active_game.save_path.string() + "\"";
                system(cmd.c_str());
#endif

#ifdef _WIN32
                std::string cmd = "explorer.exe \"" + active_game.save_path.string() + "\"";
                system(cmd.c_str());
#endif
            }

            ImGui::EndChild();
        }

        if(open_restore_modal) {
            open_restore_modal = false;
            state.backups = Features::get_backups(*pending_restore_game, config); 
            if(state.backups.empty()) {
                open_restore_modal = false;
            }
            ImGui::OpenPopup("Restore Backup");
        }
        if(open_delete_modal) {
            open_delete_modal = false;
            state.backups = Features::get_backups(*pending_delete_game, config); 
            if(state.backups.empty()) {
                open_delete_modal = false;
            }
            ImGui::OpenPopup("Delete Backup");
        }

        if(ImGui::BeginPopupModal("Restore Backup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::SetNextItemWidth(550.0f);
            if(ImGui::BeginListBox("##state.backups")) {
                for(int i = 0; i < state.backups.size(); i++) {
                    if(ImGui::Selectable(state.backups[i].filename().string().c_str(), state.selected_backup_idx == i)) {
                        state.selected_backup_idx = i;
                    }
                }
                ImGui::EndListBox();
            }

            if(ImGui::Button("Restore") && !state.backups.empty()) {
                Features::restore_backup(state.backups[state.selected_backup_idx], *pending_restore_game);
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
            if(ImGui::BeginListBox("##state.backups")) {
                for(int i = 0; i < state.backups.size(); i++) {
                    if(ImGui::Selectable(state.backups[i].filename().string().c_str(), state.selected_backup_idx == i)) {
                        state.selected_backup_idx = i;
                    }
                }
                ImGui::EndListBox();
            }

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
            if(ImGui::Button("Delete") && !state.backups.empty()) {
                if(fs::remove(state.backups[state.selected_backup_idx])) {
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

    return std::nullopt;
}
