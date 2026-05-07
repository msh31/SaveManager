#include "backup_view.hpp"
#include "backend/features/backup/backup.hpp"
#include "backend/utils/paths.hpp"
#include <frontend/ui/notifications/notification.hpp>

static constexpr float button_spacing = 4.0f;
static constexpr float btn_width = 80.0f;
static constexpr const char* spinner = "|/-\\";

void BackupTab::add_new_entry(Detection::DetectionResult& d_result) {
    std::lock_guard lock_em_up(b_mutex);
    std::shared_lock lock(d_result.d_mutex);
    std::unordered_map<std::string, fs::path> save_path_lookup;
    for(const auto& game : d_result.games)
        save_path_lookup[sanitize_filename(game.game_name)] = game.save_path;
    lock.unlock();

    backups.clear();
    for (const auto& entry : fs::directory_iterator(paths::backup_dir())) {
        if(!entry.is_directory()) continue;
        BackupEntry bentry;
        bentry.name = entry.path().filename().string();
        if(auto it = save_path_lookup.find(bentry.name.string()); it != save_path_lookup.end())
            bentry.save_path = it->second;

        for (const auto& entry_b : fs::directory_iterator(entry)) {
            if(entry_b.path().filename().extension() != ".zip") continue;
            bentry.entries.push_back(entry_b.path());
            bentry.size += fs::file_size(entry_b.path());
        }

        if(bentry.entries.empty()) continue;
        backups.push_back(bentry);
    }
}

void BackupTab::render(const Fonts& fonts, Detection::DetectionResult& d_result, Config& cfg) {
    ZoneScopedN("backup_tab_render");

    ImGui::BeginChild("##backup_view", ImVec2(0, ImGui::GetContentRegionAvail().y), false, ImGuiWindowFlags_NoBackground);

    bool is_refreshing = refresh_future.valid() && refresh_future.wait_for(std::chrono::seconds(0)) != std::future_status::ready;
    if (refresh_future.valid() && refresh_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
        refresh_future.get();
    }

    std::vector<BackupEntry> snapshot;
    {
        std::lock_guard lock(b_mutex);
        snapshot = backups;
    }

    if(!is_refreshing) {
        if(ImGui::Button("Refresh")) {
            refresh_future = std::async(std::launch::async, [this, &result = d_result]() { 
                    add_new_entry(result); 
                    });
        }
        ImGui::SetItemTooltip("Re-runs the detection logic to find new backups");

        if(snapshot.empty() || reload_backups) {
            refresh_future = std::async(std::launch::async, [this, &result = d_result]() { 
                    add_new_entry(result); 
                    });
            reload_backups = false;
        }
    } else {
        int index = (spinner_frame / 10) % 4;
        ImGui::Text("%c", spinner[index]);
    }

    for (const auto& entry : snapshot) {
        render_game_row(fonts, entry, cfg);
        ImGui::Dummy(ImVec2(0, 6.0f));
    }

    render_modals(cfg);
    ImGui::EndChild();
}

void BackupTab::render_game_row(const Fonts& fonts, const BackupEntry& bentry, Config& cfg) {
    ZoneScopedN("backup_tab_game_row_render");
    bool& not_collapsed = card_collapsed[bentry.name.string()];
    bool& bk_collapsed = backups_collapsed[bentry.name.string()];
    const char* chevron = not_collapsed ? "▼" : "▶";
    const char* chevron_b = bk_collapsed ? "▶" : "▼";

    auto labels = Features::load_labels(bentry.name.string(), cfg);
    auto selectable_id = std::format("##gamename_{}", bentry.name.string());

    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(198/255.f, 97/255.f, 63/255.f, 1.f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
    ImGui::BeginChild(selectable_id.c_str(), ImVec2(0, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);
    ImGui::PopStyleColor();

    if(ImGui::Selectable("##header", false, ImGuiSelectableFlags_None, ImVec2(0, 30))) {
        not_collapsed = !not_collapsed;
    }
    ImGui::SameLine(8.0f);

    ImGui::PushFont(fonts.bold);
    ImGui::TextColored(ImColor(198, 97, 63).Value, "%s", chevron);
    ImGui::PopFont();
    ImGui::SameLine();

    ImGui::PushFont(fonts.medium);
    std::string left_text = std::format("{}", bentry.name.string());
    ImGui::Text("%s", left_text.c_str());
    ImGui::PopFont();
    std::string right_text = std::format("{} backups", bentry.entries.size());
    ImGui::SameLine(ImGui::GetContentRegionMax().x - ImGui::CalcTextSize(right_text.c_str()).x);
    ImGui::Text("%s", right_text.c_str());

    if (not_collapsed) {
        for (const auto& entry : bentry.entries) {
            render_backup_row(entry, bentry.save_path, labels, bentry.name.string(), cfg);
        }
    }

    ImGui::EndChild();
    ImGui::PopStyleVar();
}

void BackupTab::render_backup_row(fs::path path, const fs::path& save_path, const std::unordered_map<std::string, std::string>& labels, const std::string& game_name, Config& cfg) {
    ZoneScopedN("render_backup_row");
    if(!fs::exists(path)) return;
    ImGui::PushID(path.string().c_str());

    auto it = labels.find(path.filename().string());
    std::string display = (it != labels.end()) ? it->second : path.filename().string();

    std::string date_text = std::format("{:%d/%m/%y %H:%M} | ", fs::last_write_time(path));
    float date_width = ImGui::CalcTextSize(date_text.c_str()).x;
    auto b_size = fs::file_size(path) / 1024;

    std::string size_text = std::format("{}KB  ", b_size);
    float size_width = ImGui::CalcTextSize(size_text.c_str()).x;

    float total_width = date_width + size_width + btn_width * 3 + button_spacing * 5;

    ImGui::Text("%s", display.c_str());
    ImGui::SameLine(ImGui::GetContentRegionMax().x - total_width);

    ImGui::TextDisabled("%s", date_text.c_str());
    ImGui::SameLine(0.0f, button_spacing);
    ImGui::TextDisabled("%s", size_text.c_str());
    ImGui::SameLine(0.0f, button_spacing);

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3.0f, 3.0f));
    if(ImGui::Button("Restore", ImVec2(btn_width, 0))) {
        if(save_path.empty()) {
            Notify::show_notification("Restore", "Cannot restore: save location unknown (no metadata).", 2000);
        } else {
            Features::restore_backup(path, save_path);
        }
    }
    ImGui::SetItemTooltip("Restore save from backup");
    ImGui::SameLine(0.0f, button_spacing);

    if(ImGui::Button("Rename", ImVec2(btn_width, 0))) {
        pending_rename_game = game_name;
        pending_rename_backup = path;
        rename_input = (it != labels.end()) ? it->second : "";
        open_rename_modal = true;
    }
    ImGui::SetItemTooltip("Rename this backup");
    ImGui::SameLine(0.0f, button_spacing);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
    if(ImGui::Button("Delete", ImVec2(btn_width, 0))) {
        if(fs::remove(path)) {
            auto mutable_labels = labels;
            mutable_labels.erase(path.filename().string());
            Features::save_labels(game_name, cfg, mutable_labels);
            reload_backups = true;
            Notify::show_notification("Backup Deletion", "Backup deleted!", 1500);
        } else {
            Notify::show_notification("Backup Deletion", "Backup could not be deleted!", 1500);
        }
    }
    ImGui::SetItemTooltip("Delete backed up savegame");

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar();
    ImGui::PopID();
}

void BackupTab::render_modals(Config& cfg) {
    if (open_rename_modal) {
        open_rename_modal = false;
        ImGui::OpenPopup("Rename Backup");
    }

    if (ImGui::BeginPopupModal("Rename Backup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("%s", pending_rename_backup.filename().string().c_str());
        ImGui::InputText("Label", &rename_input);
        if (ImGui::Button("Save")) {
            Features::save_label(pending_rename_game, cfg,
                                 pending_rename_backup.filename().string(), rename_input);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}
