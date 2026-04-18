#include "editor_view.hpp"
#include "frontend/ui/notifications/notification.hpp"
#include <nfd.h>


void EditorTab::render(const Fonts& fonts) {
    ImGui::BeginChild("##sa_editor", ImVec2(0, ImGui::GetContentRegionAvail().y), false,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    if(ImGui::Button("Open savefile")) {
        NFD_Init();

        nfdu8char_t *outPath;
        nfdu8filteritem_t filters[2] = { { "GTA SAN ANDREAS SAVE FILE", "b" } };
        nfdopendialogu8args_t args = {0};
        args.filterList = filters;
        args.filterCount = 1;
        nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args);
        if (result == NFD_OKAY) {
            std::string path(outPath);
            NFD_FreePathU8(outPath);
            if(san_andreas.open(path)) {
                file_path = path;
                Notify::show_notification("Save Editor", "Save loaded successfully!", 3000);
            } else {
                Notify::show_notification("Save Editor", "Save failed to load!", 3000);
            }
        }
        else if (result == NFD_CANCEL) {
            // puts("User pressed cancel.");
        }
        else {
            Notify::show_notification("Save Editor", "Save failed to load!", 3000);
        }

        NFD_Quit();
    }
    ImGui::SameLine();
    if(ImGui::Button("Save")) {
        san_andreas.save(file_path);
        Notify::show_notification("Save Editor", "Savegame changed saved succesfully!", 3000);
    }


    if(!file_path.empty()) {
        float window_width = ImGui::GetWindowSize().x;
        float window_height = ImGui::GetContentRegionAvail().y;
        float top_height = 250.0f;
        float half = (window_width - 20.0f) / 2.0f;

        ImGui::BeginChild("#SA_INFO", ImVec2(half, 200.0f), true);
        ImGui::PushFont(fonts.bold);
        ImGui::Text("Information");
        ImGui::PopFont();
        ImGui::InputText("Save Path", &file_path);

        ImGui::Text("Save Name: %s", san_andreas.save_name.c_str());
        ImGui::Text("Save Version: %s", san_andreas.save_version.c_str());
        ImGui::EndChild();

        ImGui::SameLine(0.0f, 10.0f);

        ImGui::BeginChild("#SA_STATS", ImVec2(half, 200.0f), true);
        ImGui::PushFont(fonts.bold);
        ImGui::Text("Statistics");
        ImGui::PopFont();
        ImGui::SetNextItemWidth(200.0f);
        if(ImGui::DragInt(std::format("Money ({})", san_andreas.money_displayed).c_str(), &san_andreas.money, 1.0f, INT32_MIN, INT32_MAX)) {
            san_andreas.money_displayed = san_andreas.money;
        }
        ImGui::SetNextItemWidth(200.0f);
        ImGui::DragFloat("Health", &san_andreas.health, 1.0f, 0.0, san_andreas.max_health);
        ImGui::SetNextItemWidth(200.0f);
        ImGui::DragFloat("Armor", &san_andreas.armor, 1.0f, 0.0, san_andreas.max_armor);
        ImGui::EndChild();

        ImGui::BeginChild("#SA_FLAGS", ImVec2(half, window_height), true);
        ImGui::PushFont(fonts.bold);
        ImGui::Text("Flags");
        ImGui::PopFont();
        ImGui::Checkbox("Lose stuff after wasted", &san_andreas.lose_stuff_after_wasted);
        ImGui::SameLine();
        ImGui::Checkbox("Lose stuff after busted", &san_andreas.lose_stuff_after_busted);
        ImGui::Separator();
        ImGui::Checkbox("Free Wasted Once", &san_andreas.free_wasted_once);
        ImGui::SameLine();
        ImGui::Checkbox("Free Busted Once", &san_andreas.free_busted_once);
        ImGui::Separator();
        ImGui::Checkbox("Infinite Run", &san_andreas.infinite_run);
        ImGui::SameLine();
        ImGui::Checkbox("Fast Reload", &san_andreas.fast_reload);
        ImGui::SameLine();
        ImGui::Checkbox("Fireproof", &san_andreas.fireproof);
        ImGui::EndChild();

        ImGui::SameLine(0.0f, 10.0f);

        ImGui::BeginChild("#SA_COLLECT", ImVec2(0, window_height), true);
        ImGui::PushFont(fonts.bold);
        ImGui::Text("Collectables");
        ImGui::PopFont();
        if(ImGui::Button("Complete Spray Tags")) std::ranges::fill(san_andreas.tag_statuses, 255);
        ImGui::SameLine();
        if(ImGui::Button("Reset Spray Tags")) std::ranges::fill(san_andreas.tag_statuses, 0);
        if(ImGui::CollapsingHeader(std::format("Spray Tags ({})", san_andreas.tag_count).c_str())) {
            for (int i = 0; i < san_andreas.tag_count; i++) {
                bool tagged = san_andreas.tag_statuses[i] > 0;
                ImGui::Checkbox(std::format("Tag {}", i).c_str(), &tagged);
                if ((i + 1) % 5 != 0) ImGui::SameLine();
            }
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
        if(ImGui::CollapsingHeader(std::format("Unique Stunt Jumps ({})", san_andreas.usj_count).c_str())) {
            for (int i = 0; i < san_andreas.usj_count; i++) {
                bool completed = san_andreas.usj_done[i] > 0;
                ImGui::Checkbox(std::format("Stunt Jump {}", i).c_str(), &completed);
                if ((i + 1) % 5 != 0) ImGui::SameLine();
            }
        }
        ImGui::EndChild();

    }
    ImGui::EndChild();
}
