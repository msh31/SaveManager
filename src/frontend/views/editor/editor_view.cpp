#include "editor_view.hpp"
#include "frontend/ui/notifications/notification.hpp"

void EditorTab::render(const Fonts& fonts) {
    ImGui::BeginChild("#SAEditor");

    static std::string path = "/mnt/data/SteamLibrary/steamapps/compatdata/12120/pfx/drive_c/users/steamuser/Documents/GTA San Andreas User Files/GTASAsf1.b";
    ImGui::InputText("Save Path", &path);

    if(ImGui::Button("Load")) {
        san_andreas.open(path);
        Notify::show_notification("Save Editor", "Save loaded succesfully!", 3000);
    }

    ImGui::Text("Save Name: %s", san_andreas.save_name.c_str());
    ImGui::Text("Save Version: %s", san_andreas.save_version.c_str());
    ImGui::Separator();
    ImGui::Text("Money: %d (%d)", san_andreas.money, san_andreas.money_displayed);
    ImGui::Text("Health: %f/%d", san_andreas.health, san_andreas.max_health);
    ImGui::Text("Armor: %f/%d", san_andreas.armor, san_andreas.max_armor);
    ImGui::Separator();
    ImGui::Checkbox("Lose stuff after wasted", &san_andreas.lose_stuff_after_wasted);
    ImGui::Checkbox("Lose stuff after busted", &san_andreas.lose_stuff_after_busted);
    ImGui::EndChild();
}
