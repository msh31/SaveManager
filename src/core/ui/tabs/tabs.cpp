#include "tabs.hpp"
#include "imgui.h"
#include "../src/helpers/utils.hpp"
#include "../src/core/backup/backup.hpp"

#include <filesystem>

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
                std::cout << "creating backup of: " << game.game_name.c_str() << "!\n";

                fs::path game_backup_dir = game.game_name;
                if(!fs::exists(backup_dir / game_backup_dir)) {
                    fs::create_directories(backup_dir / game_backup_dir);
                }

                fs::path zip_name = backup_dir / game_backup_dir / construct_backup_name(game);

                Backup::create_backup(zip_name, game);
            }
            ImGui::SameLine();
            if(ImGui::Button("Restore")) {
                if(fs::is_empty(backup_dir)) {
                    std::cerr << "No backups were found! Create some first...\n";
                    return;
                }

                std::cout << "This feature still needs porting over to the GUI version..\n";
                // std::cout << "restoring backup of: " << game.game_name.c_str() << "!\n";
            }

            ImGui::EndChild();
            ImGui::SameLine(); 
        }
    } else {
        ImGui::Text("None of the supported games were found on your system!");
    }
}

void Tabs::render_log_tab(const Fonts& fonts) {

}

void Tabs::render_about_tab(const Fonts& fonts) {
    ImGui::PushFont(fonts.title);
    ImGui::Text("SaveManager");
    ImGui::PopFont();
}

