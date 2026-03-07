#include "notification.hpp"
#include "imgui.h"
#include <chrono>

std::vector<Notify::Notification> notifications;

void Notify::show_notification(const std::string& title, const std::string& text, int duration_ms) {

    Notify::Notification notification;
    notification.title = title;
    notification.text = text;
    notification.duration = duration_ms / 1000.f;
    notification.start_time = std::chrono::steady_clock::now();

    notifications.push_back(notification);
}

void Notify::render_notifications() {
    float offset = 10.f;
    for (int i = 0; i < notifications.size(); i++) {
        Notification& notif = notifications[i];
        float elapsed = std::chrono::duration<float>(std::chrono::steady_clock::now() - notif.start_time).count();

        if(elapsed > notif.duration) {
            notifications.erase(notifications.begin() + i);
            --i;
        } else {
            float notif_width = 220.f;
            ImGui::SetNextWindowPos(ImVec2(
                ImGui::GetIO().DisplaySize.x - notif_width - 10.f,
                offset
            ), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(notif_width, 0), ImGuiCond_Always);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 0.9f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.f, 10.f));

            std::string window_id = "Notification##" + std::to_string(i);
            ImGui::Begin(window_id.c_str(), nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs);
            ImGui::Text("%s", notif.title.c_str());
            ImGui::Separator();
            ImGui::TextWrapped("%s", notif.text.c_str());
            notif.last_height = ImGui::GetWindowSize().y;
            ImGui::End();
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor();

            offset += notif.last_height + 10.f;
        }
    }
}
