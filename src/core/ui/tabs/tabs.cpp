#include "tabs.hpp"
#include "core/features/features.hpp"
#include "core/helpers/paths.hpp"

#include "core/ui/notifications/notification.hpp"
#include "imgui.h"

bool open_restore_modal = false;
std::vector<fs::path> backups;
const Game* pending_restore_game = nullptr;
int selected_backup_idx = 0;
static double last_read_time = 0.0;

void Tabs::render_general_tab(const Fonts& fonts, const Detection::DetectionResult& result, std::unordered_map<std::string, GLuint> texture_id, Config& config) {
    ImGui::PushFont(fonts.header);
    ImGui::Text("Detected Games");
    ImGui::PopFont();

    int count = 0;
    float available_width = ImGui::GetContentRegionAvail().x;
    int columns = (std::max)(1, (int)(available_width / (300 + 10))); //10 = gap
    if(!result.games.empty()) {
        for (const auto& game : result.games) {
            if(count >= columns) {
                ImGui::NewLine();
                count = 0;
            }

            if(count > 0) {
                ImGui::SameLine(0.0f, 10.0f);
            }

            count++;
            ImGui::BeginChild(game.game_name.c_str(), ImVec2(300, 300), true);
            ImGui::TextWrapped("%s", game.game_name.c_str());
            ImGui::Separator();

            ImGui::Dummy(ImVec2(0.0f, 8.0f));

            auto it = texture_id.find(game.appid);
            int texture;
            if(it != texture_id.end()) {
                texture = it->second;
                ImGui::Image((ImTextureID)texture, ImVec2(280, 131));
            }
            else {
                ImGui::Text("An image was not found :(");
            }

            ImGui::Dummy(ImVec2(0.0f, 8.0f));

            if(ImGui::Button("Backup")) {
                Features::backup_game(game, config);
            }
            ImGui::SameLine();
            if(ImGui::Button("Restore")) {
                pending_restore_game = &game;
                open_restore_modal = true;
            }

            ImGui::EndChild();
        }

        if(open_restore_modal) {
            ImGui::OpenPopup("Restore Backup");
            open_restore_modal = false;
            backups = Features::get_backups(*pending_restore_game, config); 
            if(backups.empty()) {
                open_restore_modal = false;
            }
        }

        if(ImGui::BeginPopupModal("Restore Backup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::SetNextItemWidth(550.0f);
            if(ImGui::BeginListBox("##backups")) {
                for(int i = 0; i < backups.size(); i++) {
                    if(ImGui::Selectable(backups[i].filename().string().c_str(), selected_backup_idx == i)) {
                        selected_backup_idx = i;
                    }
                }
                ImGui::EndListBox();
            }

            if(ImGui::Button("Restore") && !backups.empty()) {
                Features::restore_backup(backups[selected_backup_idx], *pending_restore_game);
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
    } else {
        ImGui::Text("None of the supported games were found on your system!");
    }
}

void Tabs::render_log_tab(const Fonts& fonts) {
    static std::string log_buffer;
    std::ifstream log_file(config_dir / "savemanager.log");

    if (ImGui::Button("Clear")) {
        std::ofstream clear_file(config_dir / "savemanager.log", std::ios::trunc);
        clear_file.close();
        log_buffer.clear();
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

void Tabs::render_about_tab(const Fonts& fonts) {
    ImGui::NewLine();

    float win_width = ImGui::GetWindowSize().x;

    ImGui::PushFont(fonts.title);
    float title_width = ImGui::CalcTextSize("SaveManager").x;
    ImGui::SetCursorPosX((win_width - title_width) * 0.5f);
    ImGui::Text("SaveManager");
    ImGui::PopFont();

    ImGui::PushFont(fonts.medium);
    const char* subtitle = "The definitive local save manager";
    float subtitle_width = ImGui::CalcTextSize(subtitle).x;
    ImGui::SetCursorPosX((win_width - subtitle_width) * 0.5f);
    ImGui::TextDisabled("%s", subtitle);
    ImGui::PopFont();

    ImGui::Dummy(ImVec2(0.0f, 4.0f));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0f, 4.0f));

    ImGui::PushFont(fonts.header);
    ImGui::Text("Details");
    ImGui::PopFont();

    ImGui::Text("Version");    ImGui::SameLine(120.0f); ImGui::Text("1.0.0");
    ImGui::Text("Author");     ImGui::SameLine(120.0f); ImGui::Text("marco007");
    ImGui::Text("License");    ImGui::SameLine(120.0f); ImGui::Text("MIT");
    ImGui::Text("Source");     ImGui::SameLine(120.0f);
    ImGui::TextLinkOpenURL("click for sauce", "https://git.marco007.dev/marco/SaveManager");

    ImGui::Dummy(ImVec2(0.0f, 4.0f));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0f, 4.0f));

    ImGui::PushFont(fonts.header);
    ImGui::Text("Description");
    ImGui::PopFont();

    ImGui::TextWrapped(
        "A tool for backing up and restoring game saves locally. "
        "Supports Steam, Ubisoft, Rockstar, and more."
    );

    ImGui::Dummy(ImVec2(0.0f, 4.0f));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0f, 4.0f));

    ImGui::PushFont(fonts.header);
    ImGui::Text("Built With");
    ImGui::PopFont();

    ImGui::Text("Dear ImGui | ");
    ImGui::SameLine();
    ImGui::Text("GLFW | ");
    ImGui::SameLine();
    ImGui::Text("OpenGL | ");
    ImGui::SameLine();
    ImGui::Text("libcurl | ");
    ImGui::SameLine();
    ImGui::Text("libzip");
}

void Tabs::render_settings_tab(const Fonts& fonts, Config& config) {
    ImGui::PushFont(fonts.header);
    ImGui::Text("Settings");
    ImGui::PopFont();
   
    ImGui::PushFont(fonts.medium);
    ImGui::Text("Launcher Support");
    ImGui::PopFont();
    ImGui::Checkbox("Ubisoft Connect", &config.settings.ubi_enabled);
    ImGui::SameLine();
    ImGui::Checkbox("Rockstar Games Launcher", &config.settings.rsg_enabled);
    ImGui::Separator();

    ImGui::PushFont(fonts.medium);
    ImGui::Text("Paths");
    ImGui::PopFont();
    static char backup_buf[256];
    static char lutris_buf[256];
    static char steam_buf[256];

    static bool initialized = false;
    if (!initialized) {
        snprintf(backup_buf, sizeof(backup_buf), "%s", config.settings.backup_path.string().c_str());
        snprintf(lutris_buf, sizeof(lutris_buf), "%s", config.settings.lutris_path.c_str());
        snprintf(steam_buf, sizeof(steam_buf), "%s", config.settings.steam_path.c_str());
        initialized = true;
    }

    ImGui::InputText("Backup path", backup_buf, sizeof(backup_buf));
    ImGui::InputText("Lutris path", lutris_buf, sizeof(lutris_buf));
    ImGui::InputText("Steam path", steam_buf, sizeof(steam_buf));
    ImGui::Separator();

    if (ImGui::Button("Save")) {
        config.settings.backup_path = fs::path(backup_buf);
        config.settings.lutris_path = lutris_buf;
        config.settings.steam_path = steam_buf;
        config.save();
        Notify::show_notification("Config Saved!", "Settings saved successfully!", 1500);
    }
}

void Tabs::render_editor_tab(const Fonts& fonts) {
    ImGui::PushFont(fonts.header);
    ImGui::Text("Save Editor");
    ImGui::PopFont();
}

void Tabs::render_debug_tab(const Fonts& fonts) {
    ImGui::PushFont(fonts.header);
    ImGui::Text("Debug menu");
    ImGui::PopFont();

    if(ImGui::Button("test notification")) {
        Notify::show_notification("test", "this is a test!", 2000);
        Notify::show_notification("test 2", "this is longer test to see how long text gets handled because I have some issues...", 3000);
    }
}
