#include "about.hpp"

void AboutTab::render(const Fonts& fonts) {
    ImGui::NewLine();

    float win_width = ImGui::GetWindowSize().x;

    ImGui::PushFont(fonts.title);
    float title_width = ImGui::CalcTextSize("SaveManager").x;
    ImGui::SetCursorPosX((win_width - title_width) * 0.5f);
    ImGui::Text("SaveManager");
    ImGui::PopFont();

    ImGui::PushFont(fonts.medium);
    float subtitle_width = ImGui::CalcTextSize(subtitle).x;
    ImGui::SetCursorPosX((win_width - subtitle_width) * 0.5f);
    ImGui::TextDisabled("%s", subtitle);
    ImGui::PopFont();

    ImGui::Dummy(ImVec2(0.0f, 4.0f));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0f, 4.0f));

    ImGui::PushFont(fonts.header);
    ImGui::Text("Details");
    ImGui::PopFont();

    ImGui::Text("Version");    ImGui::SameLine(120.0f); ImGui::Text("1.2.0");
    ImGui::Text("Author");     ImGui::SameLine(120.0f); ImGui::Text("marco007");
    ImGui::Text("License");    ImGui::SameLine(120.0f); ImGui::Text("GPLv3");
    ImGui::Text("Source");     ImGui::SameLine(120.0f);
    ImGui::TextLinkOpenURL("click for sauce", "https://github.com/msh31/SaveManager");

    ImGui::Dummy(ImVec2(0.0f, 4.0f));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0f, 4.0f));

    ImGui::PushFont(fonts.header);
    ImGui::Text("Description");
    ImGui::PopFont();

    ImGui::TextWrapped(
        "A tool for backing up and restoring game saves locally and over sftp. "
        "Supports Steam, Lutris, Unreal, Heroic, Ubisoft, Rockstar, and more."
    );

    ImGui::Dummy(ImVec2(0.0f, 4.0f));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0f, 4.0f));

    ImGui::PushFont(fonts.header);
    ImGui::Text("Built With");
    ImGui::PopFont();

    ImGui::Text("Dear ImGui | ");
    ImGui::SameLine();
    ImGui::Text("GLFW | ");
    ImGui::SameLine();
    ImGui::Text("OpenGL | ");
    ImGui::SameLine();
    ImGui::Text("libcurl | ");
    ImGui::SameLine();
    ImGui::Text("libzip | ");
    ImGui::SameLine();
    ImGui::Text("libssh2 | ");
    ImGui::SameLine();
    ImGui::Text("openssl");
}
