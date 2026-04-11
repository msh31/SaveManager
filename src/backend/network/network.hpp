#pragma once
#include <curl/curl.h>

namespace Network {
    size_t write_callback(void* ptr, size_t size, size_t nmemb, FILE* stream);
    size_t stream_callback(void* ptr, size_t size, size_t nmemb, FILE* stream);

    bool download_file(std::string_view url, const std::string& output_path);
    bool is_update_available();

    void download_game_image(std::string_view appid);

    std::string download_to_string(std::string_view url);
};
