#include "backup.hpp"
#include "frontend/ui/notifications/notification.hpp"
#include "backend/logger/logger.hpp"
#include "backend/config/config.hpp"
#include "backend/utils/utils.hpp"
#include "backend/utils/zip_archive/zip_archive.hpp"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

void Features::backup_game(const Game& game, Config& config) {
    ZoneScopedN("backup_game");
    get_logger().info("creating backup of: {}", game.game_name);
    fs::path game_backup_dir = config.settings.backup_path / sanitize_filename(game.game_name);

    if(!fs::exists(game_backup_dir)) {
        fs::create_directories(game_backup_dir);
    }

    fs::path zip_name = game_backup_dir / construct_backup_name(game);

    ZipArchive archive(MODE_CREATE_ARCHIVE, zip_name.u8string());
    if(!archive.add_to_archive(game)) {
        Notify::show_notification("Backup Creation", "Failed to create backup! Please refer to the logfile!", 2000);
    } else {
        Notify::show_notification("Backup created!", std::format("A backup has been created: {}!", game.game_name), 2000);
    }
}

std::vector<fs::path> Features::get_backups(const Game& game, Config& config) {
    ZoneScopedN("get_backups");
    fs::path game_backup_dir = config.settings.backup_path / sanitize_filename(game.game_name);

    if(!fs::exists(game_backup_dir)) {
        // get_logger().error("No backups found for: {}", sanitize_filename(game.game_name));
        return {};
    }

    auto backups = fs::recursive_directory_iterator(game_backup_dir)
        | std::views::filter([](const auto& e) {
            return e.is_regular_file() && e.path().extension() == ".zip";
        })
        | std::views::transform(&fs::directory_entry::path)
        | std::ranges::to<std::vector>();

    return backups;
}

void Features::restore_backup(const fs::path& name, const Game& selected_game) {
    ZoneScopedN("restore_backup");
    ZipArchive archive(MODE_EXTRACT_ARCHIVE, name.u8string());
    if(!archive.extract_archive(selected_game)) {
        Notify::show_notification("Backup Extraction", "Failed to restore backup! Please refer to the logfile!", 2000);
    } else {
        Notify::show_notification("Backup restored!", std::format("The backup: {} has been restored!", name.string()), 2000);
    }
}

std::string Features::construct_backup_name(const Game& game, const std::string& custom_name) {
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::format("{:%Y%m%d_%H%M%S}", now);

    std::string game_name = sanitize_filename(game.game_name);
    std::string game_name_sanitized = space2underscore(game_name);
    std::string filename = custom_name;

    if(filename.empty()) {
        filename = game_name_sanitized;
    }
    return std::format("backup_{}_{}.zip", filename, timestamp);
}

std::unordered_map<std::string, std::string> Features::load_labels(const Game& game, Config& config) {
    ZoneScopedN("load_labels");
    json data;
    std::unordered_map<std::string, std::string> backup_labels;
    std::string file_name = (config.settings.backup_path / game.game_name / "labels.json").string();
    std::ifstream file(file_name.c_str());

    if(!fs::exists(file_name)) {
        return {};
    }

    if (file.is_open()) {
        data = json::parse(file);

        for (const auto& entry : data.items()) {
            backup_labels[entry.key()] = entry.value().get<std::string>();
        }
        return backup_labels;
    } else {
        get_logger().error("Failed to open labels to load it!");
    }

    return {};
}

void Features::save_label(const Game& game, Config& config, const std::string& filename, const std::string& label) {
    std::string file_name = (config.settings.backup_path / game.game_name / "labels.json").string();
    auto labels = load_labels(game, config);

    labels[filename] = label;

    json data;
    for (const auto& [key, value] : labels) {
        data[key] = value;
    }

    std::ofstream out(file_name);
    out << data.dump(4);
}


void Features::save_labels(const Game& game, Config& config, const std::unordered_map<std::string, std::string>& labels) {
    std::string file_name = (config.settings.backup_path / game.game_name / "labels.json").string();

    json data;
    for (const auto& [key, value] : labels) {
        data[key] = value;
    }

    if(data.empty()) {
        fs::remove(file_name);
    } else {
        std::ofstream out(file_name);
        out << data.dump(4);
    }
}
