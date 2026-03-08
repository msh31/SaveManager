#include "features.hpp"

void Features::backup_game(const Game& game, Config& config) {
    get_logger().info("creating backup of: " + game.game_name);
    fs::path game_backup_dir = config.settings.backup_path / game.game_name;

    if(!fs::exists(game_backup_dir)) {
        fs::create_directories(game_backup_dir);
    }

    fs::path zip_name = game_backup_dir / construct_backup_name(game);
    create_backup(zip_name, game);
    Notify::show_notification("Backup created", "A backup has been created for: " + game.game_name, 2500);
}

std::vector<fs::path> Features::get_backups(const Game& game, Config& config) {
    std::vector<fs::path> backups;
    fs::path game_backup_dir = config.settings.backup_path / game.game_name;

    if(!fs::exists(game_backup_dir)) {
        get_logger().error("No backups found for: " + game.game_name);
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
    int zip_error;
    std::string utf8_path = name.u8string();
    zip_t* archive = zip_open(utf8_path.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &zip_error);

    if(!archive) {
        get_logger().error("Could not create backup!");
        return;
    }

    int file_count = 0;
    for (const auto& entry : fs::recursive_directory_iterator(selected_game.save_path)) {
        if (entry.is_regular_file()) {
            std::string entry_utf8 = entry.path().u8string();
            fs::path relative = fs::relative(entry_utf8.c_str(), selected_game.save_path);
            get_logger().info("Adding: " + relative.string() + " to the backup for " + selected_game.game_name);

            zip_source_t* source = zip_source_file(archive, entry_utf8.c_str(), 0, 0);
            if (!source) {
                get_logger().error("Failed to create source for: " + entry_utf8);
                continue;
            }

            if (zip_file_add(archive, relative.string().c_str(), source, ZIP_FL_OVERWRITE) < 0) {
                get_logger().error("Failed to add file: " + std::string(zip_strerror(archive)));
            }
            file_count++;
        }
    }
    get_logger().success("Added " + std::to_string(file_count) + " files");
    get_logger().success("backup has been created!");
    zip_close(archive);
}

void Features::restore_backup(const fs::path& name, const Game& selected_game) {
    int zip_error;
    std::vector<std::string> failed_files;

    std::string selected_backup_utf8 = name.u8string();
    zip_t* archive = zip_open(selected_backup_utf8.c_str(), 0, &zip_error);

    if(!archive) {
        get_logger().error("Could not open backup for restoration process!");
        return;
    }

    int file_count = zip_get_num_entries(archive, 0);
    for (int i = 0; i < file_count; i++) {
        struct zip_stat fileInfo;
        zip_stat_init(&fileInfo); 

        if (zip_stat_index(archive, i, 0, &fileInfo) == 0) {
            get_logger().info(std::string("File Name: ") + fileInfo.name);
            const auto& output_path = selected_game.save_path / fileInfo.name;
            get_logger().info("Saving to: " + output_path.string());

            zip_file* file = zip_fopen_index(archive, i, 0);

            if (!file) {
                get_logger().warning("Failed to open file in archive: " + std::string(fileInfo.name));
                failed_files.push_back(fileInfo.name);
                continue;
            }
            char buffer[1024];

            zip_int64_t bytes_read;
            if(!fs::create_directories(output_path.parent_path())) {
                failed_files.push_back(fileInfo.name);
                continue;
            }
            std::ofstream save_file(output_path, std::ios::binary);

            if (!save_file.is_open()) {
                get_logger().error("Failed to open save file for writing: " + output_path.string());
                failed_files.push_back(fileInfo.name);
                zip_fclose(file);
                continue;
            }

            while ((bytes_read = zip_fread(file, buffer, sizeof(buffer))) > 0) {
                save_file.write(buffer, bytes_read);
            }
            zip_fclose(file);
        }
    }

    zip_close(archive);
    if (!failed_files.empty()) {
        Notify::show_notification("Restore failed!", "The backup: " + selected_backup_utf8 + " could not be restored!", 5000);
        get_logger().error("Failed to restore:");
        for (const auto& f : failed_files) {
            get_logger().error("  - " + f);
        }
    } else {
        get_logger().success("backup for: " + selected_game.game_name + " has been restored!");
        Notify::show_notification("Backup restored!", "The backup: " + selected_backup_utf8 + " has been restored!", 2500);
    }
}


std::string Features::construct_backup_name(const Game& game, const std::string& custom_name) {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    char time_buf[20];
    std::strftime(time_buf, sizeof(time_buf), "%Y%m%d_%H%M%S", &tm);
    std::string game_name = space2underscore(game.game_name);
    std::string filename = custom_name;

    if(filename.empty()) {
        filename = space2underscore(game.game_name);
    }

    return "backup_" + filename + "_" + std::string(time_buf) + ".zip";
}
