#include <utils/zip_archive/zip_archive.hpp>
#include <logger/logger.hpp>
#include <string>
#include <utils/utils.hpp>

#include <nlohmann/json.hpp>
#include <zip.h>

using json = nlohmann::json;

bool ZipArchive::add_to_archive(const fs::path& file) {
    int file_count = 0;
    std::vector<std::string> failed_files;
    std::vector<fs::path> save_files;

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
        save_files.push_back(file);
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
                failed_files.push_back(entry.path().filename());
                zip_source_free(source);
            }
            file_count++;
            save_files.push_back(file_path);
        }
    }

    if (!failed_files.empty()) {
        get_logger().error("Failed to backup:");
        for (const auto& f : failed_files) {
            get_logger().error("  - {}", f);
        }
        return false;
    } else {
        manifest = build_manifest(save_files);
        if(manifest.empty()) {
            get_logger().error("Empty manifest, aborting backup!");
            return false;
        }
        if(!write_manifest_to_zip(archive)) {
            get_logger().error("Failed to add checksum manifest to backup!");
            return false;
        }

        get_logger().success("backup for: {} has been created!", file.parent_path().string());
        return true;
    }
}

bool ZipArchive::extract_archive(const fs::path& save_path) {
    if(archive == nullptr) return false;

    int file_count = zip_get_num_entries(archive, 0);
    std::vector<std::string> failed_files;
    
    if(!read_manifest_from_zip(archive)) {
        get_logger().warning("Failed to read file manifest, proceed with caution!");
    }

    json manifest_json;
    try {
        manifest_json = json::parse(manifest);
    } catch(json::exception& ex) {
        get_logger().error("manifest parsing error: {}", ex.what());
    }

    for (int i = 0; i < file_count; i++) {
        struct zip_stat fileInfo;
        zip_stat_init(&fileInfo); 
        if (zip_stat_index(archive, i, 0, &fileInfo) == 0) {
            if(fileInfo.name && std::string(fileInfo.name) == "checksum.json") continue;

            get_logger().info("File Name: {}", fileInfo.name);
            const auto& output_path = save_path / fileInfo.name;
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
            save_file.close();
            if(!manifest_json.contains(fileInfo.name)) continue;
            if((hash_file(output_path).compare(manifest_json[fileInfo.name].get<std::string>())) != 0) {
                get_logger().warning("{}'s hash does not match {}'s hash, proceed with caution!", output_path.filename().string(), manifest_json[fileInfo.name].get<std::string>());
            }
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

void ZipArchive::set_comment(const std::string& str) {
    zip_set_archive_comment(archive, str.c_str(), str.size());
}

const char* ZipArchive::get_comment() {
    int len = 0;
    return zip_get_archive_comment(archive, &len, 0);
}

std::string ZipArchive::build_manifest(std::vector<fs::path> paths) {
    json data;
    std::vector<fs::path> failed_files;

    for(const auto& entry : paths) {
        auto hash = hash_file(entry);
        if(hash.empty()) { 
            failed_files.emplace_back(entry.filename());
            continue;
        }
        data[entry.filename().string()] = hash;
    }
    
    if(!failed_files.empty()) { //TODO: improve this
        get_logger().error("Failed to hash a file, aborting..");
        return {};
    }
    if(data.empty()) {
        get_logger().error("Failed to add hash to manifest");
        return {};
    }
    return data.dump();
}

bool ZipArchive::write_manifest_to_zip(zip_t* archive) {
    if(archive == nullptr) {
        get_logger().error("invalid archive");
        return false;
    }

    zip_source_t* source = zip_source_buffer(archive, manifest.data(), manifest.size(), 0);
    if(source == nullptr) {
        get_logger().error("Failed to create source for manifest");
        return false;
    }

    if(zip_file_add(archive, "checksum.json", source, ZIP_FL_OVERWRITE) < 0) {
        get_logger().error("Failed to add manifest to zip");
        zip_source_free(source);
        return false;
    }

    return true;
}

bool ZipArchive::read_manifest_from_zip(zip_t* archive) {
    if(archive == nullptr) {
        get_logger().error("invalid archive");
        return false;
    }

    auto entries = zip_get_num_entries(archive, ZIP_FL_UNCHANGED);
    std::string checksum_file = "checksum.json";
    bool found = false;
    json data;

    for (size_t i {}; i < entries; i++) {
        std::string found_file = zip_get_name(archive, i, ZIP_FL_UNCHANGED);
        if ((found_file.compare(checksum_file)) != 0) continue;

        zip_file* file = zip_fopen_index(archive, i, 0);
        if (file == nullptr) {
            get_logger().warning("Failed to open manifest in archive");
            continue;
        }

        struct zip_stat fileInfo;
        zip_stat_init(&fileInfo); 
        if (zip_stat_index(archive, i, 0, &fileInfo) == 0) {
            std::string content(fileInfo.size, '\0');
            zip_fread(file, content.data(), fileInfo.size);
            zip_fclose(file);
            data = json::parse(content);
            found = true;
        }
    }

    if (!found) get_logger().warning("checksum.json not found in archive");
    manifest = data.dump();
    return true;
}
