#pragma once
#include <backend/utils/utils.hpp>
#include <backend/config/config.hpp>
#include "backend/detection/detection.hpp"

struct BackupTab {
    struct BackupEntry {
        fs::path name;
        fs::path save_path;
        std::vector<fs::path> entries;
        size_t size;
    };

    void render(const Fonts&, Detection::DetectionResult&, Config&);
    std::vector<BackupEntry> backups;
private:
    std::future<void> refresh_future;
    std::unordered_map<std::string, bool> card_collapsed;
    std::unordered_map<std::string, bool> backups_collapsed;

    std::string pending_rename_game;
    fs::path pending_rename_backup;
    std::string rename_input;
    bool open_rename_modal = false;
    bool reload_backups = false;

    int spinner_frame = 0;

    void render_game_row(const Fonts& fonts, const BackupEntry& bentry, Config& cfg);
    void render_backup_row(fs::path path, const fs::path& save_path, const std::unordered_map<std::string, std::string>& labels, const std::string& game_name, Config& cfg);
    void render_modals(Config& cfg);
    void add_new_entry(Detection::DetectionResult& d_result);
};
