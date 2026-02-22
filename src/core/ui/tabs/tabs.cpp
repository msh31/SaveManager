#include "tabs.hpp"

void Tabs::render_general_tab(const Fonts& fonts, const Detection::DetectionResult& result) {
    ImGui::Text("test");

    for (const auto& game : result.games) {
        ImGui::BeginChild(game.game_name.c_str(), ImVec2(200, 150), true);
        ImGui::Text("%s", game.game_name.c_str());
        // buttons etc
        ImGui::EndChild();
        ImGui::SameLine(); 
    }
}

void Tabs::render_log_tab(const Fonts& fonts) {

}

void Tabs::render_about_tab(const Fonts& fonts) {
    ImGui::PushFont(fonts.title);
    ImGui::Text("SaveManager");
    ImGui::PopFont();
}
