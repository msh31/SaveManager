#pragma once
#include <curl/curl.h>

namespace Network {
    size_t write_callback(void* ptr, size_t size, size_t nmemb, FILE* stream);
    bool download_file(const std::string_view& url, const std::string& output_path);
    bool download_game_image(const std::string& appid);
};
