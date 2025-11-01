#include "ui_manager.hpp"

// member initializer list
// initiialzes those variables bebfore the constructor runs, like magic
UIManager::UIManager()
    : selectedGameIndex(-1)
    , selectedProfileIndex(0)
    , needsProfileSelection(false)
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

            auto ids = profile::detectUserIds();
            for (const auto& id : ids) {
                ImGui::Text("Profile ID found: %s\n", id.c_str());
            }

            if (ImGui::Button("test setup modal")) {
                ImGui::OpenPopup("Setup");
            }
            ImGui::SameLine();
// profile selection modal
            if(ImGui::Button("test profile modal") || needsProfileSelection) {
                ImGui::OpenPopup("Select A Profile");
            }

            if (ImGui::BeginPopupModal("Select A Profile", NULL, ImGuiWindowFlags_NoResize)) {
                ImGui::Text("Select a ubisoft account profile:");

                ImGui::Separator();
                ImGui::Dummy(ImVec2(0, 2));

                const char* preview = detectedProfiles.empty() ? "No profiles"
                                    : detectedProfiles[selectedProfileIndex].c_str();

                if (ImGui::BeginCombo("##profile", preview)) {
                    for (int n = 0; n < detectedProfiles.size(); n++) {
                        const bool is_selected = (selectedProfileIndex == n);

                        if (ImGui::Selectable(detectedProfiles[n].c_str(), is_selected)) {
                            selectedProfileIndex = n;
                        }

                        if (is_selected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }

                if (ImGui::Button("Confirm")) {
                    needsProfileSelection = false;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
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

std::string UIManager::getSelectedProfile() {
    if (selectedProfileIndex >= 0 && selectedProfileIndex < detectedProfiles.size()) {
        return detectedProfiles[selectedProfileIndex];
    }
    return "";
}

bool UIManager::hasValidSelection() {
    return !needsProfileSelection && !detectedProfiles.empty() && selectedProfileIndex >= 0;
}
