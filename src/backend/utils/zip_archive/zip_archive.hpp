#pragma once
#include <zip.h>
#include "backend/logger/logger.hpp"
#include <types.hpp>

#define MODE_CREATE_ARCHIVE (ZIP_CREATE | ZIP_TRUNCATE)
#define MODE_EXTRACT_ARCHIVE 0

class ZipArchive {
public:
    ZipArchive(int mode, fs::path name) {
        int zip_error;
        archive = zip_open(name.string().c_str(), mode, &zip_error);

        if(!archive) {
            get_logger().error("Failed to open archive: {}", name.string());
        }
    }

    ~ZipArchive() {
        if(archive != nullptr) {
            zip_close(archive);
        }
    }

    bool add_to_archive(const fs::path& file);
    bool extract_archive(const fs::path& save_path);

    // disable copying (prevent accidental double-cleanup)
    ZipArchive(const ZipArchive&) = delete;
    ZipArchive& operator=(const ZipArchive&) = delete;

private:
    zip_t* archive = nullptr;
};
