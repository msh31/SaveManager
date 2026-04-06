#pragma once

struct SanAndreas {
    std::vector<uint8_t> data;
    std::unordered_map<int, size_t> block_offsets;
    static constexpr int save_size = 202752;
    static constexpr int block_count = 34;
    inline static const std::unordered_map<std::string, std::string> version_strings = {
        {"75,81,da,35", "Version 1.00 Unmodified"},
        {"83,e5,f3,65", "Version 1.00 Modified"},
        {"58,be,6e,9a", "Version 1.01 Unmodified"},
        {"5e,76,45,93", "Version 1.01 Modified"},
        {"f6,8d,14,fd", "Version 2.00 Unmodified"},
        {"22,cc,31,5d", "Version 2.00 (German)"}
    };

    std::uint32_t calculate_checksum();
    bool validate_checksum();
    bool validate_file(fs::path file);
    void find_block_offsets(size_t start_offset = 0);
    std::string get_version_string(size_t offset);

    bool load(fs::path path);
};
