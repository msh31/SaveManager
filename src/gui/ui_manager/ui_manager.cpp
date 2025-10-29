#include "ui_manager.hpp"

// member initializer list
// initiialzes those variables bebfore the constructor runs, like magic
UIManager::UIManager()
    : currentTab(0)
    , selectedGameIndex(-1)
{}

UIManager::~UIManager() {

}

void UIManager::Render(ImGuiWindowFlags window_flags) {
    ImGui::Begin("SaveManager", nullptr, window_flags);

    if (ImGui::BeginTabBar("MainTabs")) {
        if (ImGui::BeginTabItem("Home")) {
            ImGui::Text("Welcome to SaveManager!");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Games")) {
            ImGui::Text("Detected games will show here");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Backups")) {
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Saves")) {
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Settings")) {
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}
