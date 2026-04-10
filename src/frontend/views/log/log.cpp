#include "log.hpp"
#include "backend/logger/logger.hpp"
#include "frontend/ui/notifications/notification.hpp"

void LogTab::render(const Fonts& fonts) {
    if (ImGui::Button("Clear")) {
        log_buffer.clear();
        get_logger().clear();
        get_logger().info("Cleared the log!");
    }
    ImGui::SameLine();
    if(ImGui::Button("Copy to clipboard")) {
        if(!log_buffer.empty()) {
            ImGui::SetClipboardText(log_buffer.c_str());
            Notify::show_notification("Logging", "Copied log to clipboard!", 2000);
        }
    }

    if (ImGui::GetTime() - last_read_time > 2.0) {
        last_read_time = ImGui::GetTime();
        log_buffer.clear();
        for (const auto& entry : get_logger().get_entries()) {
            log_buffer += entry + "\n";
        }
    }

    ImGui::TextUnformatted(log_buffer.c_str());
}
