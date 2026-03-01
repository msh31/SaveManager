#include "tabs.hpp"
#include "core/features/features.hpp"
#include "core/helpers/network.hpp"
#include "core/helpers/utils.hpp"

#include "imgui.h"

bool open_restore_modal = false;
std::vector<fs::path> backups;
const Game* pending_restore_game = nullptr;
int selected_backup_idx = 0;

void Tabs::render_general_tab(const Fonts& fonts, const Detection::DetectionResult& result, std::unordered_map<std::string, GLuint> texture_id) {
    ImGui::PushFont(fonts.header);
    ImGui::Text("Detected Games");
    ImGui::PopFont();

    int count = 0;
    if(!result.games.empty()) {
        for (const auto& game : result.games) {
            if(count >= 5) {
                ImGui::NewLine();
                count = 0;
            }

            if(count > 0) {
                ImGui::SameLine(0.0f, 10.0f);
            }

            count++;
            ImGui::BeginChild(game.game_name.c_str(), ImVec2(300, 270), true);
            ImGui::TextWrapped("%s", game.game_name.c_str());
            ImGui::Separator();

            ImGui::Dummy(ImVec2(0.0f, 8.0f));

            auto it = texture_id.find(game.appid);
            int texture;
            if(it != texture_id.end()) {
                texture = it->second;
                ImGui::Image((ImTextureID)texture, ImVec2(280, 131));
            }
            else {
                ImGui::Text("An image was not found :(");
            }

            ImGui::Dummy(ImVec2(0.0f, 8.0f));

            if(ImGui::Button("Backup")) {
                Features::backup_game(game);
            }
            ImGui::SameLine();
            if(ImGui::Button("Restore")) {
                pending_restore_game = &game;
                open_restore_modal = true;
            }

            ImGui::EndChild();
        }

        if(open_restore_modal) {
            ImGui::OpenPopup("Restore Backup");
            open_restore_modal = false;
            backups = Features::get_backups(*pending_restore_game); 
            if(backups.empty()) {
                open_restore_modal = false;
            }
        }

        if(ImGui::BeginPopupModal("Restore Backup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
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
    } else {
        ImGui::Text("None of the supported games were found on your system!");
    }
}

void Tabs::render_log_tab(const Fonts& fonts) {
    ImGui::Text("the log tab");
}

void Tabs::render_about_tab(const Fonts& fonts) {
    ImGui::PushFont(fonts.title);
    ImGui::Text("SaveManager");
    ImGui::PopFont();
}

