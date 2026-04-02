#include "log.hpp"
#include "core/logger/logger.hpp"
#include "core/ui/notifications/notification.hpp"

void LogTab::render(const Fonts& fonts) {
    std::ifstream log_file(paths::config_dir() / "savemanager.log");

    if (ImGui::Button("Clear")) {
        std::ofstream clear_file(paths::config_dir() / "savemanager.log", std::ios::trunc);
        clear_file.close();
        log_buffer.clear();
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

        if (log_file.is_open()) {
            std::string buffer;
            log_buffer.clear();
            while (std::getline(log_file, buffer)) {
                log_buffer += buffer + "\n";
            }
        }
        else {
            ImGui::Text("the log could not be opened.");
        }
    }

    ImGui::TextUnformatted(log_buffer.c_str());
    // ImGui::SetScrollHereY(1.0f);
}
