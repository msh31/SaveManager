#include "zip_archive.hpp"
#include <utils/utils.hpp>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

bool CZipArchive::add_to_archive( const fs::path& file, std::optional<std::string> parent ) {
    std::vector<std::string> failed_files;

    if ( fs::is_regular_file( file ) ) {
        if ( m_archive == nullptr ) return false;

        std::string entry_name;
        if ( parent.has_value( ) ) {
            entry_name = ( fs::path( parent.value( ) ) / file.filename( ) ).string( );
        } else {
            entry_name = file.filename( ).string( );
        }

        zip_source_t* source = zip_source_file( m_archive, file.string( ).c_str( ), 0, 0 );
        if ( source == nullptr ) {
            SPDLOG_ERROR( "Failed to create source for: {}", file.filename( ).string( ).c_str( ) );
            failed_files.push_back( file.filename( ).string( ).c_str( ) );
        } else {
            if ( zip_file_add( m_archive, entry_name.c_str( ), source, ZIP_FL_OVERWRITE ) < 0 ) {
                SPDLOG_ERROR( "Failed to add file: {}", zip_strerror( m_archive ) );
                failed_files.push_back( file.filename( ).string( ) );
                zip_source_free( source );
            } else {
                m_save_files.emplace_back( file, entry_name );
            }
        }
    }

    if ( fs::is_directory( file ) ) {
        if ( m_archive == nullptr ) return false;

        for ( const auto& entry :
              fs::recursive_directory_iterator( file, fs::directory_options::skip_permission_denied ) ) {
            if ( !fs::is_regular_file( entry ) ) continue;
            zip_source_t* source = zip_source_file( m_archive, entry.path( ).string( ).c_str( ), 0, 0 );
            if ( source == nullptr ) {
                SPDLOG_ERROR( "Failed to create source for: {}", entry.path( ).filename( ).string( ).c_str( ) );
                failed_files.push_back( entry.path( ).filename( ).string( ).c_str( ) );
            } else {
                auto file_path = fs::relative( entry.path( ), file );

                std::string zip_name = { };
                if ( parent.has_value( ) ) {
                    zip_name = ( fs::path( parent.value( ) ) / file_path ).string( );
                } else {
                    zip_name = file_path.string( );
                }

                if ( zip_file_add( m_archive, zip_name.c_str( ), source, ZIP_FL_OVERWRITE ) < 0 ) {
                    SPDLOG_ERROR( "Failed to add file: {}", zip_strerror( m_archive ) );
                    failed_files.push_back( entry.path( ).filename( ).string( ) );
                    zip_source_free( source );
                } else {
                    m_save_files.emplace_back( entry.path( ), zip_name );
                }
            }
        }
    }

    if ( !failed_files.empty( ) ) {
        SPDLOG_ERROR( "Failed to backup:" );
        for ( const auto& f : failed_files ) {
            SPDLOG_ERROR( "  - {}", f );
        }
        return false;
    } else {
        SPDLOG_INFO( "backup for: {} has been created!", file.parent_path( ).filename( ).string( ) );
        return true;
    }
}

bool CZipArchive::finalize_add( ) {
    m_manifest = build_manifest( m_save_files );

    if ( m_manifest.empty( ) ) {
        SPDLOG_ERROR( "Empty manifest, aborting backup!" );
        return false;
    }

    if ( !write_manifest_to_zip( m_archive, m_manifest ) ) {
        SPDLOG_ERROR( "Failed to add manifest to backup!" );
        return false;
    }

    if ( !close( ) ) {
        SPDLOG_ERROR( "Failed to close!" );
        return false;
    }
    return true;
}

// Manifest not being able to be parsed is fine, it might not exist
// which is the case for old backups created before this was added
bool CZipArchive::extract_archive(
    const std::vector<fs::path>& save_paths, std::vector<std::pair<fs::path, fs::path>>& conflicts,
    bool has_index_prefixes, std::unordered_set<std::string> exclusions ) {
    if ( m_archive == nullptr ) return false;
    if ( save_paths.empty( ) ) {
        SPDLOG_ERROR( "No save paths found to extract !" );
        return false;
    }

    auto manifest = read_manifest_from_zip( m_archive );

    int file_count = zip_get_num_entries( m_archive, 0 );
    std::vector<std::string> failed_files;

    json manifest_json;
    if ( manifest.has_value( ) ) {
        try {
            manifest_json = json::parse( *manifest );
        } catch ( json::exception& ex ) {
            SPDLOG_ERROR( "manifest parsing error: {}", ex.what( ) );
        }
    }

    for ( int i = 0; i < file_count; i++ ) {
        struct zip_stat fileInfo;
        zip_stat_init( &fileInfo );
        if ( zip_stat_index( m_archive, i, 0, &fileInfo ) == 0 ) {
            if ( fileInfo.name == NULL ) {
                SPDLOG_WARN( "Failed to get filename, skipping this file" );
                continue;
            }
            if ( std::string( fileInfo.name ) == "manifest.json" ) continue;
            if ( exclusions.contains( fileInfo.name ) ) continue;

            fs::path safe_base = { };
            try {
                std::string name = fileInfo.name;
                auto slash_pos = name.find( '/' );
                std::string index_str = { };
                std::string relative_name = { };

                if ( has_index_prefixes ) {
                    if ( slash_pos != std::string::npos ) {
                        index_str = name.substr( 0, slash_pos );
                        relative_name = name.substr( slash_pos + 1 );
                    } else {
                        relative_name = name;
                        index_str = { };
                    }

                    if ( !index_str.empty( ) ) {
                        size_t idx = std::stoul( index_str );
                        if ( idx < save_paths.size( ) ) {
                            safe_base = fs::weakly_canonical( save_paths[idx] );
                        } else {
                            safe_base = fs::weakly_canonical( save_paths[0] );
                        }
                    } else {
                        safe_base = fs::weakly_canonical( save_paths[0] );
                    }
                } else {
                    relative_name = name;
                    safe_base = fs::weakly_canonical( save_paths[0] );
                }

                zip_file* file = zip_fopen_index( m_archive, i, 0 );
                if ( file == nullptr ) {
                    SPDLOG_WARN( "Failed to open file in archive: {}", fileInfo.name );
                    failed_files.push_back( fileInfo.name );
                    continue;
                }

                auto resolved = fs::weakly_canonical( safe_base / relative_name );
                if ( fs::relative( resolved, safe_base ).string( ).starts_with( ".." ) ) {
                    SPDLOG_WARN( "zip-slip attempt: {}", fileInfo.name );
                    zip_fclose( file );
                    continue;
                }
                auto resolved_tmp = resolved.string( ) + ".tmp";

                char buffer[1024];
                zip_int64_t bytes_read;
                fs::create_directories( resolved.parent_path( ) );

                if ( fs::exists( resolved_tmp ) ) {
                    fs::remove( resolved_tmp );
                }

                fs::path conflict_path = { };
                if ( fs::exists( resolved ) ) {
                    SPDLOG_WARN( "{} already exists in your game directory!", resolved.filename( ).string( ) );

                    auto save_time =
                        std::chrono::system_clock::to_time_t( file_time_to_sys( fs::last_write_time( resolved ) ) );

                    auto conflict_dest =
                        resolved.parent_path( ) /
                        std::format( "{}.savemgr-conflict-{}", resolved.filename( ).string( ), save_time );

                    if ( save_time > fileInfo.mtime ) {
                        SPDLOG_WARN( "{} is newer than {}!", resolved.filename( ).string( ), fileInfo.name );
                        std::error_code ec;
                        fs::rename( resolved, conflict_dest, ec );
                        if ( ec ) {
                            SPDLOG_ERROR( "rename failed: {}", ec.message( ) );
                            continue;
                        }
                        conflict_path = conflict_dest;
                    }
                }

                auto restore_conflict = [&] {
                    if ( !conflict_path.empty( ) ) {
                        std::error_code ec;
                        fs::rename( conflict_path, resolved, ec );
                        if ( ec ) {
                            SPDLOG_WARN(
                                "Rename from {} to {} failed: {}", conflict_path.string( ), resolved.string( ),
                                ec.message( ) );
                        }
                    }
                };

                std::ofstream save_file( resolved_tmp, std::ios::binary );
                if ( !save_file.is_open( ) ) {
                    SPDLOG_ERROR( "Failed to open save file for writing: {}", resolved.filename( ).string( ) );
                    failed_files.push_back( fileInfo.name );
                    zip_fclose( file );
                    continue;
                }

                bool write_failed = false;
                while ( ( bytes_read = zip_fread( file, buffer, sizeof( buffer ) ) ) > 0 ) {
                    if ( !save_file.write( buffer, bytes_read ) ) {
                        write_failed = true;
                        break;
                    }
                }
                if ( write_failed ) {
                    zip_fclose( file );
                    failed_files.push_back( fileInfo.name );
                    fs::remove( resolved_tmp );
                    restore_conflict( );

                    continue;
                }
                if ( bytes_read == -1 ) {
                    SPDLOG_ERROR( "Failed to read file in archive: {}", fileInfo.name );
                    failed_files.push_back( fileInfo.name );
                    fs::remove( resolved_tmp );
                    restore_conflict( );
                    continue;
                }
                zip_fclose( file );
                save_file.close( );

                if ( manifest_json.contains( fileInfo.name ) ) {
                    auto& entry = manifest_json[fileInfo.name];
                    if ( ( hash_file( resolved_tmp ).compare( entry["hash"].get<std::string>( ) ) ) != 0 ) {
                        SPDLOG_WARN(
                            "{}'s hash does not match {}'s hash, aborting restore operation!",
                            resolved.filename( ).string( ), entry["hash"].get<std::string>( ) );

                        failed_files.emplace_back( fileInfo.name );
                        fs::remove( resolved_tmp );

                        restore_conflict( );
                        continue;
                    }
                    fs::rename( resolved_tmp, resolved );

                    if ( entry.contains( "mtime" ) ) {
                        auto mtime =
                            sys_to_file_time( std::chrono::system_clock::from_time_t( entry["mtime"].get<time_t>( ) ) );
                        fs::last_write_time( resolved, mtime );
                    }
                } else {
                    fs::rename( resolved_tmp, resolved );
                }
            } catch ( std::exception& ex ) {
                SPDLOG_WARN( "Error on '{}': {}", fileInfo.name, ex.what( ) );
            }
        }
    }

    if ( !failed_files.empty( ) ) {
        SPDLOG_ERROR( "Failed to restore:" );
        for ( const auto& f : failed_files ) {
            SPDLOG_ERROR( "  - {}", f );
        }
        return false;
    }
    return true;
}

void CZipArchive::set_comment( const std::string& str ) {
    zip_set_archive_comment( m_archive, str.c_str( ), str.size( ) );
}

std::string CZipArchive::get_comment( ) {
    int len = 0;
    auto comment = zip_get_archive_comment( m_archive, &len, 0 );
    if ( comment != NULL ) {
        return comment;
    }
    return { };
}

std::string CZipArchive::hash_file( const std::filesystem::path& path ) {
    if ( !std::filesystem::is_regular_file( path ) ) return { };

    EVP_MD_CTX* mdctx = EVP_MD_CTX_new( );
    if ( mdctx == nullptr ) {
        EVP_MD_CTX_free( mdctx );
        return { };
    }

    const EVP_MD* md = EVP_get_digestbyname( "SHA-256" );
    if ( md == nullptr ) {
        EVP_MD_CTX_free( mdctx );
        return { };
    }

    if ( !EVP_DigestInit_ex( mdctx, md, NULL ) ) {
        EVP_MD_CTX_free( mdctx );
        return { };
    }

    unsigned char hash[SHA256_DIGEST_LENGTH];
    unsigned int hash_len = 0;
    char buffer[8192];

    std::ifstream file( path, std::ios::binary );
    if ( !file.is_open( ) ) {
        EVP_MD_CTX_free( mdctx );
        return { };
    }

    while ( file.read( buffer, sizeof( buffer ) ) ) {
        EVP_DigestUpdate( mdctx, buffer, file.gcount( ) );
    }
    if ( file.gcount( ) > 0 ) {
        EVP_DigestUpdate( mdctx, buffer, file.gcount( ) );
    }
    file.close( );

    if ( !EVP_DigestFinal_ex( mdctx, hash, &hash_len ) ) {
        EVP_MD_CTX_free( mdctx );
        return { };
    }
    EVP_MD_CTX_free( mdctx );

    std::stringstream ss;
    for ( int i = 0; i < SHA256_DIGEST_LENGTH; i++ ) {
        ss << std::hex << std::setw( 2 ) << std::setfill( '0' ) << static_cast<int>( hash[i] );
    }
    return ss.str( );
}

std::string CZipArchive::build_manifest( std::vector<std::pair<fs::path, fs::path>> paths ) {
    json data;
    std::vector<fs::path> failed_files;

    for ( const auto& entry : paths ) {
        auto hash = hash_file( entry.first );
        if ( hash.empty( ) ) {
            failed_files.emplace_back( entry.second );
            continue;
        }

        try {
            auto save_time =
                std::chrono::system_clock::to_time_t( file_time_to_sys( fs::last_write_time( entry.first ) ) );
            data[entry.second.string( )] = { { "hash", hash }, { "mtime", save_time } };
        } catch ( std::exception& ex ) {
            failed_files.emplace_back( entry.second );
            SPDLOG_ERROR( "Failed to get last write time for {}, skipping.. ({})", entry.second.string( ), ex.what( ) );
            continue;
        }
    }

    if ( !failed_files.empty( ) ) { // TODO: improve this
        SPDLOG_ERROR( "Failed to hash a file, aborting.." );
        return { };
    }
    if ( data.empty( ) ) {
        SPDLOG_ERROR( "Failed to add hash to manifest" );
        return { };
    }
    return data.dump( );
}

bool CZipArchive::write_manifest_to_zip( zip_t* zip_handle, const std::string& manifest ) {
    if ( zip_handle == nullptr ) {
        SPDLOG_ERROR( "invalid archive" );
        return false;
    }

    zip_source_t* source = zip_source_buffer( zip_handle, manifest.data( ), manifest.size( ), 0 );
    if ( source == nullptr ) {
        SPDLOG_ERROR( "Failed to create source for manifest" );
        return false;
    }

    if ( zip_file_add( zip_handle, "manifest.json", source, ZIP_FL_OVERWRITE ) < 0 ) {
        SPDLOG_ERROR( "Failed to add manifest to zip" );
        zip_source_free( source );
        return false;
    }

    return true;
}

std::optional<std::string> CZipArchive::read_manifest_from_zip( zip_t* zip_handle ) {
    if ( zip_handle == nullptr ) {
        SPDLOG_ERROR( "invalid archive" );
        return std::nullopt;
    }

    auto entries = zip_get_num_entries( zip_handle, ZIP_FL_UNCHANGED );
    if ( entries <= 0 ) {
        SPDLOG_ERROR( "Failed to find file entries from archive!" );
        return std::nullopt;
    }

    for ( size_t i{ }; i < entries; i++ ) {
        const char* name = zip_get_name( zip_handle, i, ZIP_FL_UNCHANGED );
        if ( name == NULL ) {
            SPDLOG_WARN( "Failed to get filename, skipping this file" );
            continue;
        }

        std::string found_file = name;
        if ( ( found_file.compare( "manifest.json" ) ) != 0 ) continue;

        zip_file* file = zip_fopen_index( zip_handle, i, 0 );
        if ( file == nullptr ) {
            SPDLOG_WARN( "Failed to open manifest in archive" );
            continue;
        }

        struct zip_stat fileInfo;
        zip_stat_init( &fileInfo );
        if ( zip_stat_index( zip_handle, i, 0, &fileInfo ) == 0 ) {
            std::string content( fileInfo.size, '\0' );
            auto bytes_read = zip_fread( file, content.data( ), fileInfo.size );
            if ( bytes_read != static_cast<zip_int64_t>( fileInfo.size ) ) {
                SPDLOG_WARN( "manifest read incomplete or failed ({} of {} bytes)", bytes_read, fileInfo.size );
            }
            zip_fclose( file );
            return content;
        }
        zip_fclose( file );
    }

    SPDLOG_WARN( "manifest.json not found in archive" );
    return std::nullopt;
}

std::vector<std::string> CZipArchive::get_entry_names( ) {
    std::vector<std::string> entry_names = { };

    auto entries = zip_get_num_entries( m_archive, ZIP_FL_UNCHANGED );
    if ( entries <= 0 ) {
        SPDLOG_ERROR( "Failed to find file entries from archive!" );
        return entry_names; // empty atp
    }

    for ( size_t i{ }; i < entries; i++ ) {
        std::string name = zip_get_name( m_archive, i, ZIP_FL_UNCHANGED );
        if ( ( name.compare( "manifest.json" ) ) == 0 ) continue;

        entry_names.emplace_back( name );
    }
    return entry_names;
}
