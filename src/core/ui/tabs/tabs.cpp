#include "tabs.hpp"
#include "imgui.h"

void Tabs::render_general_tab(const Fonts& fonts, const Detection::DetectionResult& result, GLuint texture_id) {
    ImGui::PushFont(fonts.header);
    ImGui::Text("Detected Games");
    ImGui::PopFont();

    if(!result.games.empty()) {
        for (const auto& game : result.games) {
            ImGui::BeginChild(game.game_name.c_str(), ImVec2(250, 300), true);
            ImGui::Text("%s", game.game_name.c_str());
            ImGui::Separator();

            ImGui::Dummy(ImVec2(0.0f, 8.0f));
            ImGui::Image((ImTextureID)(intptr_t)texture_id, ImVec2(300, 200));
            ImGui::Dummy(ImVec2(0.0f, 8.0f));

            if(ImGui::Button("Backup")) {
                std::cout << "creating backup of: " << game.game_name.c_str() << "!\n";
            }
            ImGui::SameLine();
            if(ImGui::Button("Restore")) {
                std::cout << "restoring backup of: " << game.game_name.c_str() << "!\n";
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

