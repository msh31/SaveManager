#include "features.hpp"
#include "core/ui/notifications/notification.hpp"

void Features::backup_game(const Game& game, Config& config) {
    get_logger().info("creating backup of: " + game.game_name);
    fs::path game_backup_dir = config.settings.backup_path / sanitize_filename(game.game_name);

    if(!fs::exists(game_backup_dir)) {
        fs::create_directories(game_backup_dir);
    }

    fs::path zip_name = game_backup_dir / construct_backup_name(game);
    create_backup(zip_name, game);
    Notify::show_notification("Backup created", "A backup has been created for: " + game.game_name, 2500);
}

std::vector<fs::path> Features::get_backups(const Game& game, Config& config) {
    std::vector<fs::path> backups;
    fs::path game_backup_dir = config.settings.backup_path / sanitize_filename(game.game_name);

    if(!fs::exists(game_backup_dir)) {
        get_logger().error("No backups found for: " + sanitize_filename(game.game_name));
        return {};
    }

    for (const auto& entry : fs::recursive_directory_iterator(game_backup_dir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".zip") {
            const auto& full_path = entry.path();
            backups.emplace_back(full_path);
        }
    }

    return backups;
}

// PRIVATE 
void Features::create_backup(const fs::path& name, const Game& selected_game) {
    ZipArchive archive(MODE_CREATE_ARCHIVE, name.u8string());
    if(!archive.add_to_archive(selected_game)) {
        Notify::show_notification("Backup Creation", "Failed to create backup! Please refer to the logfile!", 2000);
    } else {
        Notify::show_notification("Backup restored!", "The backup for: " + selected_game.game_name + " has been created!", 2000);
    }
}

void Features::restore_backup(const fs::path& name, const Game& selected_game) {
    ZipArchive archive(MODE_EXTRACT_ARCHIVE, name.u8string());
    if(!archive.extract_archive(selected_game)) {
        Notify::show_notification("Backup Extraction", "Failed to restore backup! Please refer to the logfile!", 2000);
    } else {
        Notify::show_notification("Backup restored!", "The backup: " + name.string() + " has been restored!", 2000);
    }
}

std::string Features::construct_backup_name(const Game& game, const std::string& custom_name) {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    char time_buf[20];
    std::strftime(time_buf, sizeof(time_buf), "%Y%m%d_%H%M%S", &tm);
    std::string game_name = sanitize_filename(game.game_name);
    std::string game_name_sanitized = space2underscore(game_name);
    std::string filename = custom_name;

    if(filename.empty()) {
        filename = game_name_sanitized;
    }

    return "backup_" + filename + "_" + std::string(time_buf) + ".zip";
}
