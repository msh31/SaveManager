#pragma once
#include <types.hpp>
#include <zip.h>

class CZipArchive {
    public:
        CZipArchive( int mode, fs::path name ) {
            int zip_error;
            m_archive = zip_open( name.string( ).c_str( ), mode, &zip_error );

            if ( !m_archive ) {
                SPDLOG_ERROR( "Failed to open archive: {}", name.string( ) );
            }
        }

        ~CZipArchive( ) { close( ); }

        bool add_to_archive( const fs::path& file );
        bool extract_archive( const fs::path& save_path, std::vector<std::pair<fs::path, fs::path>>& conflicts );

        void        set_comment( const std::string& );
        std::string get_comment( );

        std::vector<std::string> get_entry_names( );

        // disable copying (prevent accidental double-cleanup)
        CZipArchive( const CZipArchive& )            = delete;
        CZipArchive& operator=( const CZipArchive& ) = delete;

    private:
        zip_t*      m_archive  = nullptr;
        std::string m_manifest = { };

        static std::string hash_file( const std::filesystem::path& path );
        std::string        build_manifest( std::vector<std::pair<fs::path, fs::path>> paths );
        bool               write_manifest_to_zip( zip_t* zip_handle );

        bool read_manifest_from_zip( zip_t* zip_handle );
        void close( ) {
            if ( m_archive != nullptr ) {
                auto res = zip_close( m_archive );
                if ( res != 0 ) SPDLOG_ERROR( "Zip close failure: {}", res );
            }
        }
};
