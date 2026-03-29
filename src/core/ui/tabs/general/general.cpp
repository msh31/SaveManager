#include "general.hpp"
#include "core/globals.hpp"
#include "core/ui/notifications/notification.hpp"
#include "core/features/features.hpp"
#include "imgui.h"

void GeneralTab::on_result_changed() {
    grouped_games = {};
    appid_to_group = {};
    pending_restore_game = nullptr;
    pending_delete_game = nullptr;
    m_state->selected_game_idx = 0;
    m_state->selected_backup_idx = 0;

    for (int i = 0; i < (int)m_result->games.size(); i++) {
        const auto& game = m_result->games[i];
        if (game.appid != "N/A") {
            auto it = appid_to_group.find(game.appid);
            if (it != appid_to_group.end()) {
                grouped_games[it->second].push_back(i);
            } else {
                appid_to_group[game.appid] = grouped_games.size();
                grouped_games.push_back({i});
            }
        } else {
            std::string key = "N/A::" + game.game_name;
            auto it = appid_to_group.find(key);
            if (it != appid_to_group.end()) {
                grouped_games[it->second].push_back(i);
            } else {
                appid_to_group[key] = grouped_games.size();
                grouped_games.push_back({i});
            }
        }
    }

    last_game_count = m_result->games.size();
}

std::optional<Detection::DetectionResult> GeneralTab::render(const Fonts& fonts, Detection::DetectionResult& result, const std::unordered_map<std::string, GLuint>& texture_id, Config& config, TabState& state) {
    m_result = &result;
    m_textures = &texture_id;
    m_config = &config;
    m_state = &state;

    spinner_frame++;

    if (refresh_future.valid() && refresh_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
        auto new_result = refresh_future.get();
        Notify::show_notification("Save refresh", "Saves refreshed!", 2000);
        return new_result;
    }

    if(last_game_count != result.games.size()) {
        on_result_changed();
    }

    ImGui::PushFont(fonts.header);
    ImGui::Text("Detected Games");
    ImGui::PopFont();

    bool is_refreshing = refresh_future.valid() && refresh_future.wait_for(std::chrono::seconds(0)) != std::future_status::ready;
    if(!is_refreshing) {
        if(ImGui::Button("Refresh")) {
            refresh_future = std::async(std::launch::async, [&config]() {
                return Detection::find_saves(config);
            });
        }
    } else {
        char spin_char = spinner[(spinner_frame / 10) % 4];

        std::string loading_text = std::string("Refreshing savegames... ") + spin_char;
        ImGui::Text("%s", loading_text.c_str());
    }

    render_cards();
    render_modals();

    return std::nullopt;
}

void GeneralTab::render_cards() {
    float available_width = ImGui::GetContentRegionAvail().x;
    float card_height = 380.0f;
    float padding = 10.0f;
    float min_card_width = 200.0f;
    float max_card_width = 300.0f;
    int min_columns = 3;

    int columns = (std::max)(min_columns, (int)(available_width / (min_card_width + padding)));
    float card_width = (available_width - (padding * (columns - 1))) / columns;
    card_width = (std::min)(card_width, max_card_width);
    columns = (std::max)(min_columns, (int)((available_width + padding) / (card_width + padding)));

    if(!grouped_games.empty()) {
        for (int gi = 0; gi < (int)grouped_games.size(); gi++) {
            const auto& group = grouped_games[gi];
            int row = gi / columns;
            int col = gi % columns;

            if(col == 0 && gi > 0) {
                ImGui::Dummy(ImVec2(0.0f, padding));
            }

            if(col > 0) {
                ImGui::SameLine(0.0f, padding);
            }

            const Game* primary = &m_result->games[group[0]];
            std::string card_id = primary->game_name + "##card" + std::to_string(gi);

            const Game& active_game = m_result->games[group[0]];

            ImGui::BeginChild(card_id.c_str(), ImVec2(card_width, card_height), true, 
                              ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
            render_card(*primary, active_game, group, gi);
            ImGui::EndChild();
        }

    } else {
        ImGui::Text("None of the supported games were found on your system!");
    }
}

void GeneralTab::render_card(const Game& primary, const Game& active_game, const std::vector<int>& group, int gi) {
    auto w_pos = ImGui::GetWindowPos();
    auto w_siz = ImGui::GetWindowSize();
    ImGuiViewport* vp = ImGui::GetMainViewport();

    auto it = m_textures->find(primary.appid);
    if(it != m_textures->end()) {
        ImGui::GetWindowDrawList()->AddImage(
            (ImTextureID)(intptr_t)it->second,
            w_pos,
            ImVec2(w_pos.x + w_siz.x, w_pos.y + image_height),
            ImVec2(0, 0),
            ImVec2(1, 1),
            IM_COL32(255, 255, 255, 255)
        );
        // ImGui::GetWindowDrawList()->AddRectFilled(
        //     w_pos,
        //     ImVec2(w_pos.x + w_siz.x, w_pos.y + 40.0f),
        //     IM_COL32(0, 0, 0, 200)
        // );
        // ImGui::SetCursorPos(ImVec2(8.0f, 8.0f));
        // ImGui::TextWrapped("%s", primary.game_name.c_str());
        // ImGui::SetCursorPos(ImVec2(0, image_height));
    }
    else {
        ImGui::TextWrapped("%s", primary.game_name.c_str());
    }

    if(it == m_textures->end()) {
        ImGui::Separator();
    }
    float button_y = w_siz.y - 46.0f;
    ImGui::SetCursorPos(ImVec2(0.0f, button_y));
    float button_spacing = 8.0f;
    float buttons_width = 32.0f * 4 + button_spacing * 3;
    ImGui::SetCursorPosX((w_siz.x - buttons_width) * 0.5f);

    if(ImGui::Button("\uf0c7", ImVec2(32.0f, 32.0f))) {
        Features::backup_game(active_game, *m_config);
    }
    ImGui::SameLine(0.0f, button_spacing);
    if(ImGui::Button("\uf2ea", ImVec2(32.0f, 32.0f))) {
        pending_restore_game = &active_game;
        open_restore_modal = true;
    }
    ImGui::SameLine(0.0f, button_spacing);
    if(ImGui::Button("\uf07b", ImVec2(32.0f, 32.0f))) {
#ifdef __linux__
        pid_t pid = fork();
        if (pid == 0) {
            execl("/usr/bin/xdg-open", "xdg-open", active_game.save_path.string().c_str(), nullptr);
            _exit(1);
        }
#endif

#ifdef _WIN32
        std::string cmd = "explorer.exe \"" + active_game.save_path.string() + "\"";
        ShellExecuteA(NULL, "open", active_game.save_path.string().c_str(), NULL, NULL, SW_SHOWDEFAULT);
#endif
    }
    ImGui::SameLine(0.0f, button_spacing);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
    if(ImGui::Button("\uf1f8", ImVec2(32.0f, 32.0f))) {
        pending_delete_game = &active_game;
        open_delete_modal = true;
    }
    ImGui::PopStyleColor(2);
}

void GeneralTab::render_modals() {
    if(open_restore_modal) {
        open_restore_modal = false;
        m_state->backups = Features::get_backups(*pending_restore_game, *m_config); 
        m_state->selected_backups.clear();
        m_state->selected_backups.resize(m_state->backups.size(), false);
        if(m_state->backups.empty()) {
            open_restore_modal = false;
        }
        ImGui::OpenPopup("Restore Backup");
    }
    if(open_delete_modal) {
        open_delete_modal = false;
        m_state->backups = Features::get_backups(*pending_delete_game, *m_config); 
        m_state->selected_backups.clear();
        m_state->selected_backups.resize(m_state->backups.size(), false);
        if(m_state->backups.empty()) {
            open_delete_modal = false;
        }
        ImGui::OpenPopup("Delete Backup");
    }

    if(ImGui::BeginPopupModal("Restore Backup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::SetNextItemWidth(550.0f);
        if(ImGui::BeginListBox("##state.backups")) {
            for(int i = 0; i < m_state->backups.size(); i++) {
                if(ImGui::Selectable(m_state->backups[i].filename().string().c_str(), m_state->selected_backup_idx == i)) {
                    m_state->selected_backup_idx = i;
                }
            }
            ImGui::EndListBox();
        }

        if(ImGui::Button("Restore") && !m_state->backups.empty()) {
            Features::restore_backup(m_state->backups[m_state->selected_backup_idx], *pending_restore_game);
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
            for(int i = 0; i < m_state->backups.size(); i++) {
                if(ImGui::Selectable(m_state->backups[i].filename().string().c_str(), m_state->selected_backup_idx == i)) {
                    m_state->selected_backup_idx = i;
                }
            }
            ImGui::EndListBox();
        }

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
        if(ImGui::Button("Delete") && !m_state->backups.empty()) {
            if(fs::remove(m_state->backups[m_state->selected_backup_idx])) {
                Notify::show_notification("Backup Deletion", "Backup deleted!", 1500);
            } else {
                Notify::show_notification("Backup Deletion", "Backup could not be deleted!", 1500);
            }
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
