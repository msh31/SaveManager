#include "features.hpp"
#include "core/helpers/utils.hpp"
#include "core/helpers/paths.hpp"

#include <filesystem>

void Features::backup_game(const Game& game) {
    std::cout << "creating backup of: " << game.game_name.c_str() << "!\n";
    fs::path game_backup_dir = backup_dir / game.game_name;

    if(!fs::exists(game_backup_dir)) {
        fs::create_directories(game_backup_dir);
    }

    fs::path zip_name = game_backup_dir / construct_backup_name(game);
    create_backup(zip_name, game);
}


std::vector<fs::path> Features::get_backups(const Game& game) {
    std::vector<fs::path> backups;
    fs::path game_backup_dir = backup_dir / game.game_name;

    if(!fs::exists(game_backup_dir)) {
        std::cerr << "No backups found for: " << game.game_name << "!\n";
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
        std::cout << COLOR_RED << "Could not create backup!\n" << COLOR_RESET;
        return;
    }

    int file_count = 0;
    for (const auto& entry : fs::recursive_directory_iterator(selected_game.save_path)) {
        if (entry.is_regular_file()) {
            std::string entry_utf8 = entry.path().u8string();
            fs::path relative = fs::relative(entry_utf8.c_str(), selected_game.save_path);
            std::cout << "Adding: " << relative << " to the backup for " << selected_game.game_name << "\n";

            zip_source_t* source = zip_source_file(archive, entry_utf8.c_str(), 0, 0);
            if (!source) {
                std::cerr << "Failed to create source for: " << entry_utf8.c_str() << "\n";
                continue;
            }

            if (zip_file_add(archive, relative.string().c_str(), source, ZIP_FL_OVERWRITE) < 0) {
                std::cerr << "Failed to add file: " << zip_strerror(archive) << "\n";
            }
            file_count++;
        }
    }
    std::cout << "Added " << file_count << " files\n";
    std::cout << "\nbackup has been created!\n";
    zip_close(archive);
}

void Features::restore_backup(const fs::path& name, const Game& selected_game) {
    int zip_error;
    std::vector<std::string> failed_files;

    std::string selected_backup_utf8 = name.u8string();
    zip_t* archive = zip_open(selected_backup_utf8.c_str(), 0, &zip_error);

    if(!archive) {
        std::cout << COLOR_RED << "Could not open backup for restoration process!\n" << COLOR_RESET;
        return;
    }

    int file_count = zip_get_num_entries(archive, 0);
    for (int i = 0; i < file_count; i++) {
        struct zip_stat fileInfo;
        zip_stat_init(&fileInfo); 

        if (zip_stat_index(archive, i, 0, &fileInfo) == 0) {
            std::cout << "File Name: " << fileInfo.name << "\n Saving to: \n";
            const auto& output_path = selected_game.save_path / fileInfo.name;
            std::cout << output_path << "\n";

            zip_file* file = zip_fopen_index(archive, i, 0);

            if (!file) {
                failed_files.push_back(fileInfo.name);
                continue;
            }
            char buffer[1024];

            zip_int64_t bytes_read;
            fs::create_directories(output_path.parent_path());
            std::ofstream save_file(output_path, std::ios::binary);

            while ((bytes_read = zip_fread(file, buffer, sizeof(buffer))) > 0) {
                save_file.write(buffer, bytes_read);
            }
            zip_fclose(file);
        }
    }

    zip_close(archive);
    if (!failed_files.empty()) {
        std::cerr << COLOR_RED << "Failed to restore:\n" << COLOR_RESET;
        for (const auto& f : failed_files) {
            std::cerr << "  - " << f << "\n";
        }
    } else {
        std::cout << "\nbackup for: " << selected_game.game_name << " has been restored!\n";
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
