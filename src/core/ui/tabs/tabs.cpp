#include "tabs.hpp"
#include "imgui.h"
#include "../src/core/features/features.hpp"
#include "../src/core/backup/backup.hpp"
#include "imgui_internal.h"

bool open_restore_modal = false;
std::vector<fs::path> backups;
const Game* pending_restore_game = nullptr;
int selected_backup_idx = 0;

void Tabs::render_general_tab(const Fonts& fonts, const Detection::DetectionResult& result, GLuint texture_id) {
    ImGui::PushFont(fonts.header);
    ImGui::Text("Detected Games");
    ImGui::PopFont();

    int count = 0;
    if(!result.games.empty()) {
        for (const auto& game : result.games) {
            if(count > 5) {
                ImGui::NewLine();
                count = 0;
            }

            count++;
            ImGui::BeginChild(game.game_name.c_str(), ImVec2(250, 300), true);
            ImGui::Text("%s", game.game_name.c_str());
            ImGui::Separator();

            ImGui::Dummy(ImVec2(0.0f, 8.0f));
            ImGui::Image((ImTextureID)(intptr_t)texture_id, ImVec2(300, 200));
            ImGui::Dummy(ImVec2(0.0f, 8.0f));

            if(ImGui::Button("Backup")) {
                Features::backup_game(game);
            }
            ImGui::SameLine();
            if(ImGui::Button("Restore")) {
                pending_restore_game = &game;
                open_restore_modal = true;
                // Features::restore_game_backup(game);
            }

            ImGui::EndChild();
            ImGui::SameLine(); 
        }

        if(open_restore_modal) {
            ImGui::OpenPopup("Restore Backup");
            open_restore_modal = false;
            backups = get_backups(*pending_restore_game); 
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
                Backup::restore_backup(backups[selected_backup_idx], *pending_restore_game);
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

