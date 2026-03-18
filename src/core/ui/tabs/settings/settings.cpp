#include "settings.hpp"

void SettingsTab::render(const Fonts& fonts, Config& config) {
    ImGui::PushFont(fonts.header);
    ImGui::Text("Settings");
    ImGui::PopFont();
   
    ImGui::PushFont(fonts.medium);
    ImGui::Text("Launcher Support");
    ImGui::PopFont();
    ImGui::Checkbox("Ubisoft Connect", &config.settings.ubi_enabled);
    ImGui::SameLine();
    ImGui::Checkbox("Rockstar Games Launcher", &config.settings.rsg_enabled);
    ImGui::SameLine();
    ImGui::Checkbox("Unreal Games (.sav saves)", &config.settings.unreal_enabled);
    ImGui::Separator();

    ImGui::PushFont(fonts.medium);
    ImGui::Text("Paths");
    ImGui::PopFont();

    static std::string backup_path {config.settings.backup_path};
    static std::string steam_path {config.settings.steam_path};
    static std::string lutris_path {config.settings.lutris_path};
    static std::string heroic_path {config.settings.heroic_path};

    ImGui::InputText("Backup path", &backup_path);
    ImGui::InputText("Steam path", &steam_path);
    ImGui::InputText("Lutris path", &lutris_path);
    ImGui::InputText("Heroic path", &heroic_path);

    ImGui::Separator();

    if (ImGui::Button("Save")) {
        config.settings.backup_path = fs::path(backup_path);
        config.settings.steam_path = steam_path;
        config.settings.lutris_path = lutris_path;
        config.settings.heroic_path = heroic_path;
        config.save();
        Notify::show_notification("Config Saved!", "Settings saved successfully!", 1500);
    }
    ImGui::SameLine();
    if(ImGui::Button("Update translations")) {
        if(!Network::download_file(ubi_translation_url, paths::ubi_translations().string())) {
            get_logger().error("Failed to download Ubisoft translations");
        }
        if(!Network::download_file(steam_translation_url, paths::steam_appids().string())) {
            get_logger().error("Failed to download Steam ID data");
        }
        Notify::show_notification("Translations", "All translations have been updated!", 2500);
    }
}
