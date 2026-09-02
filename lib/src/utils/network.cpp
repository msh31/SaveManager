#include <branding.hpp>
#include <charconv>
#include <curl/curl.h>
#include <logger.hpp>
#include <nlohmann/json.hpp>
#include <utils/network.hpp>

namespace fs = std::filesystem;

using json = nlohmann::json;
using curl_handle = std::unique_ptr<CURL, decltype( &curl_easy_cleanup )>;

curl_handle make_easy_handle( const char* url ) {
    curl_handle handle( curl_easy_init( ), &curl_easy_cleanup );
    if ( !handle ) {
        SPDLOG_ERROR( "[Network] Failed to initialize CURL" );
        return handle;
    }
    curl_easy_setopt( handle.get( ), CURLOPT_USERAGENT, APP_NAME );
    curl_easy_setopt( handle.get( ), CURLOPT_URL, url );
    curl_easy_setopt( handle.get( ), CURLOPT_NOSIGNAL, 1L ); //dns timeouts
    curl_easy_setopt( handle.get( ), CURLOPT_TIMEOUT, 30L );        // compelte within 30sec
    curl_easy_setopt( handle.get( ), CURLOPT_CONNECTTIMEOUT, 10L ); // connect within 10sec
    return handle;
}

size_t Network::write_callback( char* ptr, size_t size, size_t nmemb, void* stream ) {
    return fwrite( ptr, size, nmemb, (FILE*)stream ) * size;
}
size_t Network::stream_callback( char* ptr, size_t size, size_t nmemb, void* userdata ) {
    ( (std::string*)userdata )->append( ptr, size * nmemb );
    return size * nmemb;
}

bool Network::download_file( const char* url, const std::string& output_path ) {
    auto handle = make_easy_handle( url );
    if ( !handle ) return false;

    std::string tmp_path = output_path + ".tmp";
    FILE* fp = fopen( tmp_path.c_str( ), "wb" );
    if ( !fp ) {
        SPDLOG_ERROR( "[Network] Failed to open file for writing: {}", tmp_path );
        return false;
    }

    curl_easy_setopt( handle.get( ), CURLOPT_WRITEFUNCTION, write_callback );
    curl_easy_setopt( handle.get( ), CURLOPT_FAILONERROR, 1L ); // http respones 400+
    curl_easy_setopt( handle.get( ), CURLOPT_WRITEDATA, fp );

    CURLcode res = curl_easy_perform( handle.get( ) );
    fclose( fp );

    if ( res != CURLE_OK ) {
        SPDLOG_ERROR( "[Network] Failed to download file: {}", curl_easy_strerror( res ) );
        fs::remove( tmp_path );
        return false;
    }

    std::error_code ec;
    fs::rename( tmp_path, output_path, ec );
    if ( ec ) {
        SPDLOG_ERROR( "[Network] Failed to move downloaded file into place: {}", ec.message( ) );
        std::error_code ecr;
        fs::remove( tmp_path, ecr );
        if ( ecr ) {
            SPDLOG_ERROR( "[Network] Failed to remove the downloaded file during cleanup", ecr.message( ) );
            return false;
        }
        return false;
    }

    return true;
}

std::string Network::download_to_string( const char* url ) {
    auto handle = make_easy_handle( url );
    if ( !handle ) return { };

    std::string data = { };
    curl_easy_setopt( handle.get( ), CURLOPT_WRITEFUNCTION, stream_callback );
    curl_easy_setopt( handle.get( ), CURLOPT_WRITEDATA, &data );
    curl_easy_setopt( handle.get( ), CURLOPT_FAILONERROR, 1L ); // http respones 400+

    CURLcode res = curl_easy_perform( handle.get( ) );
    if ( res != CURLE_OK ) {
        SPDLOG_ERROR( "[Network] Failed to stream: {}", curl_easy_strerror( res ) );
        return { };
    }

    return data;
}

std::tuple<int, int, int> Network::parse_version( std::string_view v ) {
    std::istringstream ss( ( std::string( v ) ) );
    std::string segment = { };
    int major = 0, minor = 0, patch = 0;

    int i = 0;
    while ( std::getline( ss, segment, '.' ) && i < 3 ) {
        
        if ( i == 0 ) if ( segment.starts_with( 'v' ) ) segment.erase( 0, 1 ); // strip 'v'

        //intentional discards of the results bedcause it will default to 0,0,0 anyway
        if ( i == 0 ) std::from_chars( segment.data( ), segment.data( ) + segment.size( ), major );
        if ( i == 1 ) std::from_chars( segment.data( ), segment.data( ) + segment.size( ), minor );
        if ( i == 2 ) std::from_chars( segment.data( ), segment.data( ) + segment.size( ), patch );
        i++;
    }
    return { major, minor, patch };
}

bool Network::is_update_available( ) {
    json data;
    std::string upstream = Network::download_to_string( update_url );

    if ( upstream.empty( ) ) {
        SPDLOG_ERROR( "[Network] Failed to get connect to GitHub API to fetch the latest version" );
        return false;
    }

    try {
        data = json::parse( upstream );
    } catch ( json::exception& ex ) {
        SPDLOG_ERROR( "[Network] JSON parsing error: {}", ex.what( ) );
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
