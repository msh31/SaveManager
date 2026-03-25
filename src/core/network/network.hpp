#pragma once
#include <curl/curl.h>

namespace Network {
    size_t write_callback(void* ptr, size_t size, size_t nmemb, FILE* stream);
    size_t stream_callback(void* ptr, size_t size, size_t nmemb, FILE* stream);

    bool download_file(const std::string_view& url, const std::string& output_path);

    void download_game_image(const std::string& appid);

    std::string download_to_string(const std::string_view& url);

    bool is_update_available();
};
