#include "save_editor.hpp"
#include "backend/logger/logger.hpp"
#include <cstring>
#include <filesystem>

std::uint32_t SanAndreas::calculate_checksum() {
    std::uint32_t sum = 0;
    for (auto i = 0; i < data.size() - 4; i++) {
        sum += data[i];
    }
    return sum;
}

bool SanAndreas::validate_checksum() {
    uint32_t saved;
    std::memcpy(&saved, data.data() + data.size() - 4, 4);
    return saved == calculate_checksum();
}

std::string SanAndreas::get_version_string(size_t offset) {
    if (offset + 4 > data.size()) {
        get_logger().error("Invalid offset for version string: {}", offset);
        return {};
    }

    std::vector<uint8_t> v_bytes = {};
    for (size_t i = {}; i < 4; i++) {
        if (offset + i >= data.size()) {
            get_logger().error("Version ID truncated");
        }
        v_bytes.push_back(data[offset + i]);
    }

    std::string version_hex = {};
    for (size_t i = 0; i < v_bytes.size(); i++) {
        if (i > 0) version_hex += ",";
        version_hex += std::format("{:02x}", v_bytes[i]);
    }

    auto it = version_strings.find(version_hex);
    if (it != version_strings.end()) {
        return it->second;
        // get_logger().debug("first: {}, second: {}", it->first, it->second);
    }
    return "Unknown Version";
}

bool SanAndreas::validate_file(fs::path file) {
    if(fs::file_size(file) != SanAndreas::save_size) {
        get_logger().error("Invalid file size, expected {} bytes but got {} bytes", SanAndreas::save_size, fs::file_size(file));
        return false;
    }

    if(block_offsets.size() != SanAndreas::block_count) {
        get_logger().error("Invalid block count, expected {} but got {}", SanAndreas::block_count, block_offsets.size());
        return false;
    }

    auto v_offset = block_offsets[0];
    if(get_version_string(v_offset) == "Unknown Version") {
        get_logger().error("Unknown game version!");
        return false;
    }
    if(!validate_checksum()) {
        get_logger().error("Invalid checksum, save file may be corrupted!");
        return false;
    }
    return true;
}

// finds the offsets of "BLOCK" sections within the save file data ( https://gtamods.com/wiki/Saves_(GTA_SA) )
void SanAndreas::find_block_offsets(size_t start_offset) {
    uint8_t block_signature[5] = {0x42, 0x4C, 0x4F, 0x43, 0x4B}; // "BLOCK"
    size_t offset = start_offset;
    size_t block_index = 0;

    while (offset < data.size() - 4) {
        int has_signature = memcmp(data.data() + offset, block_signature, 5);

        if(has_signature == 0) {
            block_offsets[block_index] = offset + 5;
            block_index++;
        }
        offset++;
    }
}

bool SanAndreas::load(fs::path path) {
    std::ifstream in(path, std::ios::binary);
    if(!in) {
        get_logger().error("Failed to open savegame!");
        return false;
    }

    data = std::vector<uint8_t>(std::istreambuf_iterator<char>(in), {});
    return true;
}
