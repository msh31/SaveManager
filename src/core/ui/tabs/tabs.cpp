#include "tabs.hpp"

void Tabs::render_general_tab(const Fonts& fonts, const Detection::DetectionResult& result) {
    ImGui::Text("test");
}

void Tabs::render_log_tab(const Fonts& fonts) {

}

void Tabs::render_about_tab(const Fonts& fonts) {
    ImGui::PushFont(fonts.title);
    ImGui::Text("SaveManager");
    ImGui::PopFont();
}
