#include "network/network.hpp"
#include "utils/paths.hpp"
#include <constants.hpp>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

size_t Network::write_callback( void* ptr, size_t size, size_t nmemb, FILE* stream ) {
    return fwrite( ptr, size, nmemb, stream ) * size;
}

size_t Network::stream_callback( void* ptr, size_t size, size_t nmemb, FILE* stream ) {
    ( (std::string*)stream )->append( (char*)ptr, size * nmemb );

    return size * nmemb;
}

std::string Network::download_to_string( std::string_view url ) {
    CURL* curl = curl_easy_init( );
    if ( !curl ) {
        SPDLOG_ERROR( "Failed to initialize CURL" );
        return { };
    }

    std::string data;
    std::string url_str{ url };
    curl_easy_setopt( curl, CURLOPT_USERAGENT, "SaveManager" );
    curl_easy_setopt( curl, CURLOPT_URL, url_str.c_str( ) );
    curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, stream_callback );
    curl_easy_setopt( curl, CURLOPT_WRITEDATA, &data );

    CURLcode res = curl_easy_perform( curl );
    curl_easy_cleanup( curl );

    if ( res != CURLE_OK ) {
        SPDLOG_ERROR( "Failed to stream: {}", curl_easy_strerror( res ) );
        return { };
    }

    return data;
}

bool Network::download_file( std::string_view url, const std::string& output_path ) {
    CURL* curl = curl_easy_init( );
    if ( !curl ) {
        SPDLOG_ERROR( "Failed to initialize CURL" );
        return false;
    }

    FILE* fp = fopen( output_path.c_str( ), "wb" );
    if ( !fp ) {
        SPDLOG_ERROR( "Failed to open file for writing: {}", output_path );
        curl_easy_cleanup( curl );
        return false;
    }

    std::string url_str{ url };
    curl_easy_setopt( curl, CURLOPT_USERAGENT, "SaveManager" );
    curl_easy_setopt( curl, CURLOPT_URL, url_str.c_str( ) );
    curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, write_callback );
    curl_easy_setopt( curl, CURLOPT_WRITEDATA, fp );

    CURLcode res = curl_easy_perform( curl );

    fclose( fp );
    curl_easy_cleanup( curl );

    if ( res != CURLE_OK ) {
        SPDLOG_ERROR( "Failed to download file: {}", curl_easy_strerror( res ) );
        return false;
    }

    return true;
}

bool Network::is_update_available( ) {
    json data;
    std::string upstream = download_to_string( "https://api.github.com/repos/msh31/SaveManager/releases/latest" );

    if ( upstream.empty( ) ) {
        SPDLOG_ERROR( "Failed to get connect to GitHub API to fetch the latest version" );
        return false;
    }

    try {
        data = json::parse( upstream );
    } catch ( json::exception& ex ) {
        SPDLOG_ERROR( "JSON parsing error: {}", ex.what( ) );
        return false;
    }

    std::string latest = data.value( "tag_name", std::string( "" ) );
    auto [maj, min, pat] = parse_version( APP_VERSION );
    auto [l_maj, l_min, l_pat] = parse_version( latest );

    if ( maj > l_maj ) return false;
    if ( maj < l_maj ) return true;
    if ( min > l_min ) return false;
    if ( min < l_min ) return true;
    if ( pat > l_pat ) return false;
    if ( pat < l_pat ) return true;
    if ( maj == l_maj && min == l_min && pat == l_pat ) return false;

    return true;
}

std::tuple<int, int, int> Network::parse_version( std::string_view v ) {
    std::istringstream ss( ( std::string( v ) ) );
    std::string segment;
    int major = 0, minor = 0, patch = 0;
    int i = 0;
    while ( std::getline( ss, segment, '.' ) && i < 3 ) {
        if ( i == 0 ) segment.erase( 0, 1 ); // strip 'v'
        if ( i == 0 ) major = std::stoi( segment );
        if ( i == 1 ) minor = std::stoi( segment );
        if ( i == 2 ) patch = std::stoi( segment );
        i++;
    }
    return { major, minor, patch };
}
