#include "ui_manager.hpp"

// member initializer list
// initiialzes those variables bebfore the constructor runs, like magic
UIManager::UIManager(Config& cfg)
    : config(cfg)
    , selectedGameIndex(-1)
{
    //ImGui::OpenPopup("Setup");
}

UIManager::~UIManager() {

}

void UIManager::Render(ImGuiWindowFlags window_flags) {
    ImGui::Begin("SaveManager", nullptr, window_flags);

    static std::vector<std::string> modalProfiles;
    static int modalSelection = 0;
    static bool modalInitialized = false;

    static bool needsStartupSelection = false;
    static bool checkedStartup = false;


    if (!checkedStartup) {
        checkedStartup = true;
        if (config.cfgData.selectedProfileID.empty()) {
            auto profiles = profile::detectUserIds();
            if (profiles.size() > 1) {
                needsStartupSelection = true;
            }
        }
    }

    if (ImGui::BeginTabBar("MainTabs")) {
        if (ImGui::BeginTabItem("Home")) {
            if (needsStartupSelection) {
                ImGui::OpenPopup("Select A Profile");
                needsStartupSelection = false;
            }

            ImGui::Text("Welcome to SaveManager!");

            // ImGui::Text("Profile: %s\n", config.cfgData.selectedProfileID.c_str());

            if (ImGui::Button("test setup modal")) {
                ImGui::OpenPopup("Setup");
            }
            ImGui::SameLine();
// profile selection modal

            if(ImGui::Button("test profile modal")) {
                ImGui::OpenPopup("Select A Profile");
            }

            if (ImGui::BeginPopupModal("Select A Profile", NULL, ImGuiWindowFlags_NoResize)) {
                if (!modalInitialized) {
                    modalProfiles = profile::detectUserIds();
                    modalSelection = 0;
                    modalInitialized = true;
                }

                ImGui::Text("Select a ubisoft account profile:");

                ImGui::Separator();
                ImGui::Dummy(ImVec2(0, 2));

                const char* preview = modalProfiles.empty() ? "No profiles"
                                    : modalProfiles[modalSelection].c_str();

                 if (ImGui::BeginCombo("##profile", preview)) {
                     for (int n = 0; n < modalProfiles.size(); n++) {
                         const bool is_selected = (modalSelection == n);

                         if (ImGui::Selectable(modalProfiles[n].c_str(), is_selected)) {
                             modalSelection = n;
                         }

                         if (is_selected) {
                             ImGui::SetItemDefaultFocus();
                         }
                     }
                     ImGui::EndCombo();
                 }

                 if (ImGui::Button("Confirm")) {
                     config.cfgData.selectedProfileID = modalProfiles[modalSelection];
                     config.save();
                     ImGui::CloseCurrentPopup();
                     modalInitialized = false;
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
