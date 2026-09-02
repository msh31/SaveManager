#pragma once

namespace {
    constexpr const char* update_url = "https://api.github.com/repos/msh31/desktop-app-template/releases/latest";
}

namespace Network {
    size_t write_callback( char* ptr, size_t size, size_t nmemb, void* stream );
    size_t stream_callback( char* ptr, size_t size, size_t nmemb, void* userdata );

    bool download_file( const char* url, const std::string& output_path );

    std::string download_to_string( const char* url );

    std::tuple<int, int, int> parse_version( std::string_view v );

    bool is_update_available( );
}; // namespace Network
