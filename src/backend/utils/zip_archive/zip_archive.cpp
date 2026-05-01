#include "zip_archive.hpp"

bool ZipArchive::add_to_archive(const fs::path& file) {
    ZoneScopedN("zip_add_to_archive");
    int file_count = 0;
    std::vector<std::string> failed_files;

    if (fs::is_regular_file(file)) {
        get_logger().info("Adding: {}, to the backup for: {}", file.string(), file.parent_path().string());

        if(archive == nullptr) return false;

        zip_source_t* source = zip_source_file(archive, file.string().c_str(), 0, 0);
        if (source == nullptr) {
            get_logger().error("Failed to create source for: {}", file.filename().string().c_str());
            failed_files.push_back(file.filename().string().c_str());
            return false;
        }

        if (zip_file_add(archive, file.filename().string().c_str(), source, ZIP_FL_OVERWRITE) < 0) {
            get_logger().error("Failed to add file: {}", zip_strerror(archive));
            failed_files.push_back(file.filename().string());
            zip_source_free(source);
        }
        file_count++;
    }

    if(fs::is_directory(file)) {
        get_logger().info("Adding: {}, to the backup for: {}", file.string(), file.parent_path().string());
        if(archive == nullptr) return false;

        for(const auto& entry : fs::recursive_directory_iterator(file, fs::directory_options::skip_permission_denied)) {
            if(!fs::is_regular_file(entry)) continue;
            zip_source_t* source = zip_source_file(archive, entry.path().string().c_str(), 0, 0);
            if (source == nullptr) {
                get_logger().error("Failed to create source for: {}", entry.path().filename().string().c_str());
                failed_files.push_back(file.filename().string().c_str());
                return false;
            }

            auto file_path = fs::relative(entry.path(), file);

            if (zip_file_add(archive, file_path.string().c_str(), source, ZIP_FL_OVERWRITE) < 0) {
                get_logger().error("Failed to add file: {}", zip_strerror(archive));
                failed_files.push_back(file.filename().string());
                zip_source_free(source);
            }
            file_count++;
        }
    }

    if (!failed_files.empty()) {
        get_logger().error("Failed to backup:");
        for (const auto& f : failed_files) {
            get_logger().error("  - {}", f);
        }
        return false;
    } else {
        get_logger().success("backup for: {} has been created!", file.parent_path().string());
        return true;
    }
}

bool ZipArchive::extract_archive(const fs::path& save_path) {
    ZoneScopedN("zip_extract_archive");
    if(archive == nullptr) return false;

    int file_count = zip_get_num_entries(archive, 0);
    std::vector<std::string> failed_files;

    for (int i = 0; i < file_count; i++) {
        struct zip_stat fileInfo;
        zip_stat_init(&fileInfo); 

        if (zip_stat_index(archive, i, 0, &fileInfo) == 0) {
            get_logger().info("File Name: {}", fileInfo.name);
            const auto& output_path = save_path.parent_path() / fileInfo.name;
            get_logger().info("Saving to: {}", output_path.string());

            zip_file* file = zip_fopen_index(archive, i, 0);

            if (file == nullptr) {
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
                if(!save_file.write(buffer, bytes_read)) {
                    zip_fclose(file);
                    return false;
                }
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
        get_logger().success("backup for: {} has been restored", save_path.string());
        return true;
    }
}

