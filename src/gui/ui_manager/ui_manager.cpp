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

    float panelWidth = ImGui::GetContentRegionAvail().x * 1.0f;
    float panelHeight = ImGui::GetContentRegionAvail().y * 1.0f;

    ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - panelWidth) * 0.5f);
    ImGui::SetCursorPosY((ImGui::GetContentRegionAvail().y - panelHeight) * 0.5f);

    ImGui::BeginChild("HostSetupPanel", ImVec2(panelWidth, panelHeight), true, ImGuiWindowFlags_AlwaysUseWindowPadding);

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

    ImGui::EndChild();
    ImGui::End();
}
