#include "zip_archive.hpp"

bool ZipArchive::add_to_archive(const Game& game) {
    int file_count = 0;
    std::vector<std::string> failed_files;

    for (const auto& entry : fs::recursive_directory_iterator(game.save_path)) {
        if (entry.is_regular_file()) {
            fs::path relative = fs::relative(entry.path(), game.save_path);
            get_logger().info("Adding: {}, to the backup for: {}", relative.string(), game.game_name);

            zip_source_t* source = zip_source_file(archive, entry.path().string().c_str(), 0, 0);
            if (!source) {
                get_logger().error("Failed to create source for: {}", entry.path().string());
                failed_files.push_back(entry.path().string());
                continue;
            }

            if (zip_file_add(archive, relative.string().c_str(), source, ZIP_FL_OVERWRITE) < 0) {
                get_logger().error("Failed to add file: {}", zip_strerror(archive));
                failed_files.push_back(entry.path().string());
            }
            file_count++;
        }
    }

    if (!failed_files.empty()) {
        get_logger().error("Failed to add to backup:");
        for (const auto& f : failed_files) {
            get_logger().error("  - {}", f);
        }
        return false;
    } else {
        get_logger().success("Added {} files", file_count);
        get_logger().success("backup for: {} has been created!", game.game_name);
        return true;
    }
}

bool ZipArchive::extract_archive(const Game& game) {
    int file_count = zip_get_num_entries(archive, 0);
    std::vector<std::string> failed_files;

    for (int i = 0; i < file_count; i++) {
        struct zip_stat fileInfo;
        zip_stat_init(&fileInfo); 

        if (zip_stat_index(archive, i, 0, &fileInfo) == 0) {
            get_logger().info("File Name: {}", fileInfo.name);
            const auto& output_path = game.save_path / fileInfo.name;
            get_logger().info("Saving to: {}", output_path.string());

            zip_file* file = zip_fopen_index(archive, i, 0);

            if (!file) {
                get_logger().warning("Failed to open file in archive: {}", fileInfo.name);
                failed_files.push_back(fileInfo.name);
                continue;
            }
            char buffer[1024];

            zip_int64_t bytes_read;
            fs::create_directories(output_path.parent_path());
            std::ofstream save_file(output_path, std::ios::binary);

            if (!save_file.is_open()) {
                get_logger().error("Failed to open save file for writing: {}", output_path.string());
                failed_files.push_back(fileInfo.name);
                zip_fclose(file);
                continue;
            }

            while ((bytes_read = zip_fread(file, buffer, sizeof(buffer))) > 0) {
                save_file.write(buffer, bytes_read);
            }
            if (bytes_read == -1) {
                get_logger().error("Failed to read file in archive: {}", fileInfo.name);
                failed_files.push_back(fileInfo.name);
            }
            zip_fclose(file);
        }
    }

    if (!failed_files.empty()) {
        get_logger().error("Failed to restore:");
        for (const auto& f : failed_files) {
            get_logger().error("  - {}", f);
        }
        return false;
    } else {
        get_logger().success("backup for: {} has been restored", game.game_name);
        return true;
    }
}

