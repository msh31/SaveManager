#include "network.hpp"
#include "core/globals.hpp"
#include "core/helpers/paths.hpp"
#include "core/logger/logger.hpp"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

size_t Network::write_callback(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    return fwrite(ptr, size, nmemb, stream);
}

size_t Network::stream_callback(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    ((std::string*)stream)->append((char*)ptr, size * nmemb);

    return size * nmemb;
}

std::string Network::download_to_string(const std::string_view& url) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        get_logger().error("Failed to initialize CURL");
        return {};
    }

    std::string data;
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "SaveManager");
    curl_easy_setopt(curl, CURLOPT_URL, std::string(url).c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, stream_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
    
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        get_logger().error("Failed to stream: " + std::string(curl_easy_strerror(res)));
        return {};
    }
    
    return data;
}

bool Network::download_file(const std::string_view& url, const std::string& output_path) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        get_logger().error("Failed to initialize CURL");
        return false;
    }
    
    FILE* fp = fopen(output_path.c_str(), "wb");
    if (!fp) { 
        get_logger().error("Failed to open file for writing: " + output_path);
        curl_easy_cleanup(curl); 
        return false; 
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, std::string(url).c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    
    CURLcode res = curl_easy_perform(curl);
    
    fclose(fp);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        get_logger().error("Failed to download file: " + std::string(curl_easy_strerror(res)));
        return false;
    }
    
    return true;
}

void Network::download_game_image(const std::string& appid) {
    std::string output_file = appid + ".jpg";
    fs::path img_path = paths::cache_dir() / output_file; 

    if (fs::exists(img_path) && fs::file_size(img_path) > 0) {
        return;
    }

    std::string url =
        "https://cdn.cloudflare.steamstatic.com/steam/apps/" +
        appid +
        "/library_600x900.jpg";

    if (!Network::download_file(url, img_path.string())) {
        get_logger().error("Could not download " + img_path.string());
    }
}

bool Network::is_update_available() {
    json data;
    std::string upstream = download_to_string("https://api.github.com/repos/msh31/SaveManager/releases/latest");

    if(upstream.empty()) {
        get_logger().error("Failed to get connect to GitHub API to fetch the latest version");
        return false;
    }

    try {
        data = json::parse(upstream);
    } catch(json::exception& ex) {
        get_logger().error(ex.what());
        return false;
    }

    std::string latest = data.value("tag_name", std::string(""));
    // get_logger().info("Latest: " + latest + " Current: " + std::string(APP_VERSION));
    if(APP_VERSION == latest || APP_VERSION >= latest) {
        return false;
    }

    return true;
}
