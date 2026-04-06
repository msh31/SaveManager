#include "editor_view.hpp"
#include "backend/logger/logger.hpp"
#include "frontend/ui/notifications/notification.hpp"

void EditorTab::render(const Fonts& fonts) {
    ImGui::BeginChild("#SAEditor");

    static std::string path = "/mnt/data/SteamLibrary/steamapps/compatdata/12120/pfx/drive_c/users/steamuser/Documents/GTA San Andreas User Files/GTASAsf1.b";
    ImGui::InputText("Save Path", &path);

    if(ImGui::Button("Load")) {
        san_andreas.load(path);
        get_logger().debug("loaded: {}", path);

        san_andreas.find_block_offsets();

        if(san_andreas.validate_file(path)) {
            get_logger().debug("file validated!");
        } else {
            get_logger().debug("file failed to validate!");
        }
        san_andreas.parse_block_zero();
        san_andreas.parse_block_fifteen();
        Notify::show_notification("Save Editor", "Save loaded succesfully!", 3000);
    }

    ImGui::Text("Save Name: %s", san_andreas.save_name.c_str());
    ImGui::Text("Save Version: %s", san_andreas.save_version.c_str());
    ImGui::Text("Money: %d (%d)", san_andreas.money, san_andreas.money_displayed);
    ImGui::EndChild();
}
