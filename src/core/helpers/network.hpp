#pragma once
#include "paths.hpp"

#include <string>
#include <curl/curl.h>

inline size_t write_callback(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    return fwrite(ptr, size, nmemb, stream);
}

inline bool download_file(const std::string& url, const std::string& output_path) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;
    
    FILE* fp = fopen(output_path.c_str(), "wb");
    if (!fp) { curl_easy_cleanup(curl); return false; }
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    
    CURLcode res = curl_easy_perform(curl);
    
    fclose(fp);
    curl_easy_cleanup(curl);
    
    return res == CURLE_OK;
}

inline bool download_ubi_translations() {
    fs::path output_path = config_dir / "gameids.json";
    return download_file(
        "https://git.marco007.dev/marco/Ubisoft-Game-Ids/raw/branch/master/gameids.json", 
        output_path.string()
    );
}
