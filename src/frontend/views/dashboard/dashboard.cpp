#include "dashboard.hpp"
#include "frontend/ui/notifications/notification.hpp"
#include "backend/features/backup/backup.hpp"

static constexpr const char* spinner = "|/-\\";

// static std::string_view platform_label(PlatformType t) {
//     switch(t) {
//         case PlatformType::Steam:   return "Steam";
//         case PlatformType::Heroic:  return "Heroic";
//         case PlatformType::Wine:    return "Wine";
//         // ...
//     }
// }
//

void DashboardTab::on_result_changed(RenderContext& ctx) {
    grouped_games = {};
    pending_restore_game = nullptr;
    pending_delete_game = nullptr;
    ctx.state.selected_game_idx = 0;
    ctx.state.selected_backup_idx = 0;
    grouped_games = ctx.result.get_grouped();
    last_game_count = ctx.result.games.size();
}

std::optional<Detection::DetectionResult> DashboardTab::render(const Fonts& fonts, Detection::DetectionResult& result, const std::unordered_map<std::string, GLuint>& texture_id, Config& config, TabState& state) {
    RenderContext ctx{result, texture_id, config, state, fonts};
    spinner_frame++;

    if (refresh_future.valid() && refresh_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
        auto new_result = refresh_future.get();
        Notify::show_notification("Save refresh", "Saves refreshed!", 2000);
        return new_result;
    }

    if(last_game_count != result.games.size()) {
        on_result_changed(ctx);
    }

    render_toolbar(ctx);
    render_game_list(ctx);
    render_modals(ctx);

    return std::nullopt;
}

void DashboardTab::render_toolbar(RenderContext& ctx) {
    ImGui::PushFont(ctx.fonts.header);
    ImGui::Text("Dashboard");
    ImGui::PopFont();

    bool is_refreshing = refresh_future.valid() && refresh_future.wait_for(std::chrono::seconds(0)) != std::future_status::ready;
    bool is_backing_up = backup_future.valid() && backup_future.wait_for(std::chrono::seconds(0)) != std::future_status::ready;

    ImGui::SetNextItemWidth(350.f);
    ImGui::InputText("##search", &search_query);
    ImGui::SameLine();

    if(!is_refreshing) {
        if(ImGui::Button("Refresh")) {
            refresh_future = std::async(std::launch::async, [&config = ctx.config]() {
                return Detection::find_saves(config);
            });
        }
        ImGui::SetItemTooltip("Re-runs the detection logic to find new saves");
    } else {
        char spin_char = spinner[(spinner_frame / 10) % 4];
        std::string loading_text = std::string("Refreshing savegames...") + spin_char;
        ImGui::Text("%s", loading_text.c_str());
    }
    ImGui::SameLine();
    if(!is_backing_up) {
        if(ImGui::Button("Mass Backup")) {
            backup_future = std::async(std::launch::async, [this, &result = ctx.result, &config = ctx.config]() {
                for (auto& entry : result.games) {
                    Features::backup_game(entry, config);
                }
            });
        }
        ImGui::SetItemTooltip("Creates a backup of all games found!");
    } else {
        char spin_char = spinner[(spinner_frame / 10) % 4];
        std::string loading_text = std::string("Creating backups...") + spin_char;
        ImGui::Text("%s", loading_text.c_str());
    }
}

void DashboardTab::render_game_list(RenderContext& ctx) {
    std::transform(search_query.begin(), search_query.end(), search_query.begin(),
                   ::tolower);

    for (auto [gi, group] : std::views::enumerate(grouped_games)) {
        const Game& primary = ctx.result.games[group[0]];
        std::string game_name = primary.game_name;

        // filter
        std::transform(game_name.begin(), game_name.end(), game_name.begin(),
                  ::tolower);
        if (!search_query.empty()) {
            if(game_name.find(search_query) == std::string::npos) {
                continue;
            }

        }
        render_game_row(ctx, group, static_cast<int>(gi));
    }
}

void DashboardTab::render_game_row(RenderContext& ctx, const std::vector<int>& group, int gi) {
    const Game& primary = ctx.result.games[group[0]];

    auto header_name = std::format("{} [{}] {} saves | {} backups ", primary.game_name.c_str(), "platform", 0, ctx.state.backups.size());
    if(ImGui::CollapsingHeader(header_name.c_str())) {
        for (const auto& index : group) {
            const Game& game = ctx.result.games[index];

            for (const auto& file : fs::directory_iterator(game.save_path, fs::directory_options::skip_permission_denied)) {
                render_save_row(ctx, file.path(), game);
            }
        }
    }
}

void DashboardTab::render_save_row(RenderContext& ctx, const fs::path& save_file, const Game& game) {
    ImGui::Text("%s", save_file.filename().string().c_str());
}

void DashboardTab::render_modals(RenderContext& ctx) {
    if(open_restore_modal) {
        open_restore_modal = false;
        ctx.state.backups = Features::get_backups(*pending_restore_game, ctx.config); 
        ctx.state.selected_backups.clear();
        ctx.state.selected_backups.resize(ctx.state.backups.size(), false);
        if(ctx.state.backups.empty()) {
            open_restore_modal = false;
        }
        ImGui::OpenPopup("Restore Backup");
    }
    if(open_delete_modal) {
        open_delete_modal = false;
        ctx.state.backups = Features::get_backups(*pending_delete_game, ctx.config); 
        ctx.state.selected_backups.clear();
        ctx.state.selected_backups.resize(ctx.state.backups.size(), false);
        if(ctx.state.backups.empty()) {
            open_delete_modal = false;
        }
        ImGui::OpenPopup("Delete Backup");
    }

    if(ImGui::BeginPopupModal("Restore Backup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::SetNextItemWidth(550.0f);
        if(ImGui::BeginListBox("##state.backups")) {
            auto labels = Features::load_labels(*pending_restore_game, ctx.config);
            for(auto [gi, backup] : std::views::enumerate(ctx.state.backups)) {
                auto it = labels.find(backup.filename().string());

                auto ftime = fs::last_write_time(backup);
                std::string date = std::format("{:%d %b %Y %H:%M}", ftime);
                auto b_size = fs::file_size(backup) / 1024;

                std::string display_name = std::format(" - {} - {}KB", date, b_size); 
                std::string display = (it != labels.end()) ? it->second + display_name : backup.filename().string();

                if(ImGui::Selectable(display.c_str(), ctx.state.selected_backup_idx == static_cast<int>(gi))) {
                    ctx.state.selected_backup_idx = static_cast<int>(gi);
                }
            }
            ImGui::EndListBox();

            ImGui::InputText("Backup Label", &label_input);
            if(ImGui::Button("Save Label")) {
                Features::save_label(*pending_restore_game, ctx.config, ctx.state.backups[ctx.state.selected_backup_idx].filename().string(), label_input);
            }
        }

        if(ImGui::Button("Restore") && !ctx.state.backups.empty()) {
            Features::restore_backup(ctx.state.backups[ctx.state.selected_backup_idx], *pending_restore_game);
            ImGui::CloseCurrentPopup();
            open_restore_modal = false;
        }
        ImGui::SameLine();
        if(ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
            pending_restore_game = nullptr;
            open_restore_modal = false;
        }
        ImGui::EndPopup();
    }
    if(ImGui::BeginPopupModal("Delete Backup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::SetNextItemWidth(550.0f);
        if(ImGui::BeginListBox("##state.backups")) {
            auto labels = Features::load_labels(*pending_delete_game, ctx.config);
            for(auto [gi, backup] : std::views::enumerate(ctx.state.backups)) {
                auto it = labels.find(backup.filename().string());

                auto ftime = fs::last_write_time(backup);
                std::string date = std::format("{:%d %b %Y %H:%M}", ftime);
                auto b_size = fs::file_size(backup) / 1024;

                std::string display_name = std::format(" - {} - {}KB", date, b_size); 
                std::string display = (it != labels.end()) ? it->second + display_name : backup.filename().string();

                if(ImGui::Selectable(display.c_str(), ctx.state.selected_backup_idx == static_cast<int>(gi))) {
                    ctx.state.selected_backup_idx = static_cast<int>(gi);
                }
            }
            ImGui::EndListBox();
        }

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
        if(ImGui::Button("Delete") && !ctx.state.backups.empty()) {
            auto labels = Features::load_labels(*pending_delete_game, ctx.config);
            if(fs::remove(ctx.state.backups[ctx.state.selected_backup_idx])) {
                Notify::show_notification("Backup Deletion", "Backup deleted!", 1500);
            } else {
                Notify::show_notification("Backup Deletion", "Backup could not be deleted!", 1500);
            }

            labels.erase(ctx.state.backups[ctx.state.selected_backup_idx].filename().string());
            Features::save_labels(*pending_delete_game, ctx.config, labels);

            ImGui::CloseCurrentPopup();
            open_delete_modal = false;
        }
        ImGui::PopStyleColor(2);
        ImGui::SameLine();
        if(ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
            pending_delete_game = nullptr;
            open_delete_modal = false;
        }
        ImGui::EndPopup();
    }
}
