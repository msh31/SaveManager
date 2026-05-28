#pragma once
#include <spdlog/spdlog.h>
#include <types.hpp>
#include <zip.h>

class ZipArchive {
    public:
    ZipArchive( int mode, fs::path name ) {
        int zip_error;
        archive = zip_open( name.string( ).c_str( ), mode, &zip_error );

        if ( !archive ) {
            SPDLOG_ERROR( "Failed to open archive: {}", name.string( ) );
        }
    }

    ~ZipArchive( ) {
        if ( archive != nullptr ) {
            zip_close( archive );
        }
    }

    bool add_to_archive( const fs::path &file );
    bool extract_archive( const fs::path &save_path, std::vector<std::pair<fs::path, fs::path>> &conflicts );

    void set_comment( const std::string & );
    const char *get_comment( );

    std::vector<std::string> get_entry_names( );

    // disable copying (prevent accidental double-cleanup)
    ZipArchive( const ZipArchive & ) = delete;
    ZipArchive &operator=( const ZipArchive & ) = delete;

    private:
    zip_t *archive = nullptr;
    std::string manifest = { };

    std::string build_manifest( std::vector<std::pair<fs::path, fs::path>> paths );
    bool write_manifest_to_zip( zip_t *archive );

    bool read_manifest_from_zip( zip_t *archive );
};
