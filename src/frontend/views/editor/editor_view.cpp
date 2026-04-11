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
    ImGui::SameLine();
    if(ImGui::Button("Save")) {
        san_andreas.save(path);
        Notify::show_notification("Save Editor", "Savegame changed saved succesfully!", 3000);
    }

    ImGui::Text("Save Name: %s", san_andreas.save_name.c_str());
    ImGui::Text("Save Version: %s", san_andreas.save_version.c_str());
    ImGui::Separator();
    ImGui::Text("Money: %d (%d)", san_andreas.money, san_andreas.money_displayed);
    ImGui::Text("Health: %f/%d", san_andreas.health, san_andreas.max_health);
    ImGui::Text("Armor: %f/%d", san_andreas.armor, san_andreas.max_armor);
    ImGui::Separator();
    ImGui::Checkbox("Lose stuff after wasted", &san_andreas.lose_stuff_after_wasted);
    ImGui::SameLine();
    ImGui::Checkbox("Lose stuff after busted", &san_andreas.lose_stuff_after_busted);
    ImGui::Separator();
    ImGui::Checkbox("Free Busted Once", &san_andreas.free_wasted_once);
    ImGui::SameLine();
    ImGui::Checkbox("Free Wasted Once", &san_andreas.free_wasted_once);
    ImGui::Separator();
    ImGui::Checkbox("Infinite Run", &san_andreas.infinite_run);
    ImGui::SameLine();
    ImGui::Checkbox("Fast Reload", &san_andreas.fast_reload);
    ImGui::SameLine();
    ImGui::Checkbox("Fireproof", &san_andreas.fireproof);
    ImGui::Separator();
    if(ImGui::Button("Complete Spray Tags")) std::ranges::fill(san_andreas.tag_statuses, 255);
    ImGui::SameLine();
    if(ImGui::Button("Reset Spray Tags")) std::ranges::fill(san_andreas.tag_statuses, 0);
    ImGui::Text("Tag count: %d", san_andreas.tag_count);
    for (int i = 0; i < san_andreas.tag_count; i++) {
        bool tagged = san_andreas.tag_statuses[i] > 0;
        ImGui::Checkbox(std::format("Tag {}", i).c_str(), &tagged);
        if ((i + 1) % 5 != 0) ImGui::SameLine();
    }
    ImGui::Separator();
    if(ImGui::Button("Complete Unique Stunt Jumps")) {
        std::ranges::fill(san_andreas.usj_found, true);
        std::ranges::fill(san_andreas.usj_done, true);
    }
    ImGui::SameLine();
    if(ImGui::Button("Reset Unique Stunt Jumps")) {
        std::ranges::fill(san_andreas.usj_found, false);
        std::ranges::fill(san_andreas.usj_done, false);
    }
    ImGui::Text("Unique Stunt Jumps: %d", san_andreas.usj_count);
    for (int i = 0; i < san_andreas.usj_count; i++) {
        bool completed = san_andreas.usj_done[i] > 0;
        ImGui::Checkbox(std::format("Stunt Jump {}", i).c_str(), &completed);
        if ((i + 1) % 5 != 0) ImGui::SameLine();
    }
    ImGui::Separator();

    ImGui::EndChild();
}
