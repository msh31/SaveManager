#include "ui_manager.hpp"

// member initializer list
// initiialzes those variables bebfore the constructor runs, like magic
UIManager::UIManager()
    : selectedGameIndex(-1)
{
    //ImGui::OpenPopup("Setup");
}

UIManager::~UIManager() {

}

void UIManager::Render(ImGuiWindowFlags window_flags) {
    ImGui::Begin("SaveManager", nullptr, window_flags);

    if (ImGui::BeginTabBar("MainTabs")) {
        if (ImGui::BeginTabItem("Home")) {
            ImGui::Text("Welcome to SaveManager!");

            if (ImGui::Button("test setup modal")) {
                ImGui::OpenPopup("Setup");
            }

            if (ImGui::BeginPopupModal("Setup", NULL, ImGuiWindowFlags_NoResize)) {
                ImGui::Text("Hi there, Welcome to SaveManager!\n\nFirst, the application will perform some profile, game and save detection.");

                ImGui::Separator();
                ImGui::Dummy(ImVec2(0, 5));

                if (ImGui::Button("Close")) {
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }

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
