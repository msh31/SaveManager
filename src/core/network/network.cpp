#include "network.hpp"

size_t Network::write_callback(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    return fwrite(ptr, size, nmemb, stream);
}

bool Network::download_file(const std::string& url, const std::string& output_path) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        logger().error("Failed to initialize CURL");
        return false;
    }
    
    FILE* fp = fopen(output_path.c_str(), "wb");
    if (!fp) { 
        logger().error("Failed to open file for writing: " + output_path);
        curl_easy_cleanup(curl); 
        return false; 
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    
    CURLcode res = curl_easy_perform(curl);
    
    fclose(fp);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        logger().error("Failed to download file: " + std::string(curl_easy_strerror(res)));
        return false;
    }
    
    return true;
}

bool Network::download_game_image(const std::string& appid) {
    std::string output_file = appid + ".jpg";
    fs::path img_path = cache_dir / output_file; 

    if (fs::exists(img_path)) {
        return true;
    }

    std::string url =
        "https://cdn.cloudflare.steamstatic.com/steam/apps/" +
        appid +
        "/header.jpg";

    return Network::download_file(url, img_path.string());
}
