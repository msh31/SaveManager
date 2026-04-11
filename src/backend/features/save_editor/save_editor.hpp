#pragma once
#include <cstdint>

struct SanAndreas {
    bool open(fs::path path);
    //block 0
    std::string save_name = {};
    std::string save_version = {};
    bool has_ever_cheated = false;
    //block 2
    float health = 0.0;
    float armor = 0.0f;
    //block 5
    bool lose_stuff_after_wasted = true; //08DD
    bool lose_stuff_after_busted = true; //08DE
    //block 15
    int32_t money = 0;
    int32_t money_displayed = 0;
    int max_health = 176; //temp
    int max_armor = 150;
    bool free_busted_once = false;
    bool free_wasted_once = false; //0414
    bool infinite_run  = false; //0330
    bool fast_reload  = false; //0331
    bool fireproof  = false; //055D
    //block 20
    uint32_t tag_count = 0;
    std::vector<uint8_t> tag_statuses;
    //block 24
    uint8_t usj_count = 0;
    std::vector<bool> usj_done {};
    std::vector<bool> usj_found {};
    
private:
    std::uint32_t calculate_checksum();
    bool validate_checksum();
    bool validate_file();
    void find_block_offsets(size_t start_offset = 0);
    std::string get_version_string(size_t offset);
    void parse_block_zero();
    void parse_block_two();
    void parse_block_five();
    void parse_block_fifteen();
    void parse_block_twenty();
    void parse_block_twenty_four();

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
};
