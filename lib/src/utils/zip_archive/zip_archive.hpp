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

        bool add_to_archive(
            const fs::path& file, std::optional<std::string> parent = std::nullopt,
            std::optional<std::string> entry_name_override = std::nullopt );
        bool extract_archive(
            const std::vector<fs::path>& save_paths, std::vector<std::pair<fs::path, fs::path>>& conflicts,
            bool has_index_prefixes, std::unordered_set<std::string> exclusions = { } );

        void set_comment( const std::string& );
        std::string get_comment( );

        std::vector<std::string> get_entry_names( );

        bool finalize_add( );

        // disable copying (prevent accidental double-cleanup)
        CZipArchive( const CZipArchive& ) = delete;
        CZipArchive& operator=( const CZipArchive& ) = delete;

    private:
        zip_t* m_archive = nullptr;
        std::string m_manifest = { };

        std::vector<std::pair<fs::path, fs::path>> m_save_files;

        static std::string hash_file( const std::filesystem::path& path );
        std::string build_manifest( std::vector<std::pair<fs::path, fs::path>> paths );
        bool write_manifest_to_zip( zip_t* zip_handle, const std::string& manifest );

        std::optional<std::string> read_manifest_from_zip( zip_t* zip_handle );
        bool close( ) {
            if ( m_archive != nullptr ) {
                auto res = zip_close( m_archive );
                m_archive = nullptr;
                return !res;
            }
            return true;
        }
};
