#include "save_editor.hpp"
#include "backend/logger/logger.hpp"

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
    if (it != version_strings.end()) return it->second;
    return "Unknown Version";
}

bool SanAndreas::validate_file() {
    if(data.size() != SanAndreas::save_size) {
        get_logger().error("Invalid file size, expected {} bytes but got {} bytes", SanAndreas::save_size, data.size());
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

bool SanAndreas::open(fs::path path) {
    std::ifstream in(path, std::ios::binary);
    if(!in) {
        get_logger().error("Failed to open savegame!");
        return false;
    }

    data = std::vector<uint8_t>(std::istreambuf_iterator<char>(in), {});

    find_block_offsets();
    if(validate_file()) {
        get_logger().debug("file validated!");
    } else {
        get_logger().debug("file failed to validate!");
        return false;
    }
    get_logger().info("parsing savefile: {}", path.filename().c_str());
    parse_block_zero();
    parse_block_two();
    parse_block_five();
    parse_block_fifteen();
    parse_block_twenty();

    return true;
}

void SanAndreas::parse_block_zero() {
    auto bz_offset = block_offsets[0];
    if(bz_offset + 0x138 > data.size()) {
        get_logger().error("Expected data length for Block 0 data was not received");
        return;
    }

    save_version = get_version_string(bz_offset);
    auto ptr = reinterpret_cast<const char*>(data.data() + bz_offset + 4);
    save_name = std::string(ptr, strnlen(ptr, 100));
}

void SanAndreas::parse_block_two() {
    auto bt_offset = block_offsets[2];
    std::memcpy(&health, data.data() + bt_offset + 0x04 + 0x1C, 4);
    std::memcpy(&armor, data.data() + bt_offset + 0x04 + 0x20, 4);
}

void SanAndreas::parse_block_five() {
    auto bf_offset = block_offsets[5];
    std::memcpy(&lose_stuff_after_wasted, data.data() + bf_offset + 0x04, 4);
    std::memcpy(&lose_stuff_after_busted, data.data() + bf_offset + 0x05, 4);
}

void SanAndreas::parse_block_fifteen() {
    auto bft_offset = block_offsets[15];
    std::memcpy(&money, data.data() + bft_offset + 4, 4);
    std::memcpy(&money_displayed, data.data() + bft_offset + 0x10, 4);
    max_health = data[bft_offset + 35];
    max_armor= data[bft_offset + 36];
}

void SanAndreas::parse_block_twenty() {
    auto bty_offset = block_offsets[20];
    std::memcpy(&tag_count, data.data() + bty_offset, 4);
    tag_statuses.resize(tag_count);
    std::memcpy(tag_statuses.data(), data.data() + bty_offset + 4, tag_count);
}

void SanAndreas::parse_block_twenty_four() {
    auto btyf_offset = block_offsets[24];
    std::memcpy(&usj_count, data.data() + btyf_offset, 4);
    usj_done.resize(usj_count);
    usj_found.resize(usj_count);

    for (uint32_t i = 0; i < usj_count; i++) {
        size_t jump_offset = btyf_offset + 4 + (i * 0x44);
        usj_done[i] = data[jump_offset + 0x40];
        usj_found[i] = data[jump_offset + 0x41];
    }
}
