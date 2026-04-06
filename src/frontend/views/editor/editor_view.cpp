#include "editor_view.hpp"
#include "backend/logger/logger.hpp"

void EditorTab::render(const Fonts& fonts) {
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
    }
}
