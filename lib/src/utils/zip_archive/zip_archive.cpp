#include <utils/utils.hpp>
#include <utils/zip_archive/zip_archive.hpp>

#include <nlohmann/json.hpp>
#include <zip.h>

using json = nlohmann::json;

// file_clock::to_sys / from_sys are not available on MSVC or Apple Clang
static std::chrono::system_clock::time_point file_time_to_sys( fs::file_time_type ft ) {
    return std::chrono::system_clock::now( ) +
           std::chrono::duration_cast<std::chrono::system_clock::duration>( ft - fs::file_time_type::clock::now( ) );
}

static fs::file_time_type sys_to_file_time( std::chrono::system_clock::time_point tp ) {
    return fs::file_time_type::clock::now( ) +
           std::chrono::duration_cast<fs::file_time_type::clock::duration>( tp - std::chrono::system_clock::now( ) );
}

bool ZipArchive::add_to_archive( const fs::path &file ) {
    int file_count = 0;
    std::vector<std::string> failed_files;
    std::vector<std::pair<fs::path, fs::path>> save_files;

    if ( fs::is_regular_file( file ) ) {
        if ( archive == nullptr ) return false;

        zip_source_t *source = zip_source_file( archive, file.string( ).c_str( ), 0, 0 );
        if ( source == nullptr ) {
            SPDLOG_ERROR( "Failed to create source for: {}", file.filename( ).string( ).c_str( ) );
            failed_files.push_back( file.filename( ).string( ).c_str( ) );
            return false;
        }

        if ( zip_file_add( archive, file.filename( ).string( ).c_str( ), source, ZIP_FL_OVERWRITE ) < 0 ) {
            SPDLOG_ERROR( "Failed to add file: {}", zip_strerror( archive ) );
            failed_files.push_back( file.filename( ).string( ) );
            zip_source_free( source );
        }
        file_count++;
        save_files.emplace_back( file, file.filename( ) );
    }

    if ( fs::is_directory( file ) ) {
        if ( archive == nullptr ) return false;

        for ( const auto &entry :
              fs::recursive_directory_iterator( file, fs::directory_options::skip_permission_denied ) ) {
            if ( !fs::is_regular_file( entry ) ) continue;
            zip_source_t *source = zip_source_file( archive, entry.path( ).string( ).c_str( ), 0, 0 );
            if ( source == nullptr ) {
                SPDLOG_ERROR( "Failed to create source for: {}", entry.path( ).filename( ).string( ).c_str( ) );
                failed_files.push_back( file.filename( ).string( ).c_str( ) );
                return false;
            }

            auto file_path = fs::relative( entry.path( ), file );

            if ( zip_file_add( archive, file_path.string( ).c_str( ), source, ZIP_FL_OVERWRITE ) < 0 ) {
                SPDLOG_ERROR( "Failed to add file: {}", zip_strerror( archive ) );
                failed_files.push_back( entry.path( ).filename( ).string( ) );
                zip_source_free( source );
            }
            file_count++;
            save_files.emplace_back( entry.path( ), file_path );
        }
    }

    if ( !failed_files.empty( ) ) {
        SPDLOG_ERROR( "Failed to backup:" );
        for ( const auto &f : failed_files ) {
            SPDLOG_ERROR( "  - {}", f );
        }
        return false;
    } else {
        manifest = build_manifest( save_files );
        if ( manifest.empty( ) ) {
            SPDLOG_ERROR( "Empty manifest, aborting backup!" );
            return false;
        }
        if ( !write_manifest_to_zip( archive ) ) {
            SPDLOG_ERROR( "Failed to add manifest to backup!" );
            return false;
        }

        SPDLOG_INFO( "backup for: {} has been created!", file.parent_path( ).filename( ).string( ) );
        return true;
    }
}

bool ZipArchive::extract_archive( const fs::path &save_path, std::vector<std::pair<fs::path, fs::path>> &conflicts ) {
    if ( archive == nullptr ) return false;

    int file_count = zip_get_num_entries( archive, 0 );
    std::vector<std::string> failed_files;

    if ( !read_manifest_from_zip( archive ) ) {
        SPDLOG_WARN( "Failed to read file manifest, proceed with caution!" );
    }

    json manifest_json;
    try {
        manifest_json = json::parse( manifest );
    } catch ( json::exception &ex ) {
        SPDLOG_ERROR( "manifest parsing error: {}", ex.what( ) );
    }

    for ( int i = 0; i < file_count; i++ ) {
        struct zip_stat fileInfo;
        zip_stat_init( &fileInfo );
        if ( zip_stat_index( archive, i, 0, &fileInfo ) == 0 ) {
            if ( fileInfo.name && std::string( fileInfo.name ) == "manifest.json" ) continue;

            // SPDLOG_INFO("File Name: {}", fileInfo.name);
            const auto &output_path = save_path / fileInfo.name;
            // SPDLOG_INFO("Saving to: {}", output_path.string());

            zip_file *file = zip_fopen_index( archive, i, 0 );

            if ( file == nullptr ) {
                SPDLOG_WARN( "Failed to open file in archive: {}", fileInfo.name );
                failed_files.push_back( fileInfo.name );
                continue;
            }

            char buffer[1024];
            zip_int64_t bytes_read;
            fs::create_directories( output_path.parent_path( ) );

            if ( fs::exists( output_path ) ) {
                SPDLOG_WARN( "{} already exists in your game directory!", output_path.filename( ).string( ) );

                auto save_time =
                    std::chrono::system_clock::to_time_t( file_time_to_sys( fs::last_write_time( output_path ) ) );

                if ( save_time > fileInfo.mtime ) {
                    SPDLOG_WARN( "{} is newer than {}!", output_path.filename( ).string( ), fileInfo.name );
                    std::error_code ec;
                    fs::rename( output_path,
                                output_path.parent_path( ) / std::format( "{}.savemgr-conflict-{}",
                                                                          output_path.filename( ).string( ),
                                                                          save_time ),
                                ec );
                    if ( ec ) SPDLOG_ERROR( "rename failed: {}", ec.message( ) );
                }
            }

            std::ofstream save_file( output_path, std::ios::binary );

            if ( !save_file.is_open( ) ) {
                SPDLOG_ERROR( "Failed to open save file for writing: {}", output_path.filename( ).string( ) );
                failed_files.push_back( fileInfo.name );
                zip_fclose( file );
                continue;
            }

            while ( ( bytes_read = zip_fread( file, buffer, sizeof( buffer ) ) ) > 0 ) {
                if ( !save_file.write( buffer, bytes_read ) ) {
                    zip_fclose( file );
                    return false;
                }
            }
            if ( bytes_read == -1 ) {
                SPDLOG_ERROR( "Failed to read file in archive: {}", fileInfo.name );
                failed_files.push_back( fileInfo.name );
            }
            zip_fclose( file );
            save_file.close( );

            if ( manifest_json.contains( fileInfo.name ) ) {
                auto &entry = manifest_json[fileInfo.name];
                if ( entry.contains( "mtime" ) ) {
                    auto mtime =
                        sys_to_file_time( std::chrono::system_clock::from_time_t( entry["mtime"].get<time_t>( ) ) );
                    fs::last_write_time( output_path, mtime );
                }

                if ( ( hash_file( output_path ).compare( entry["hash"].get<std::string>( ) ) ) != 0 ) {
                    SPDLOG_WARN( "{}'s hash does not match {}'s hash, proceed with caution!",
                                 output_path.filename( ).string( ), entry["hash"].get<std::string>( ) );
                }
            }
        }
    }

    if ( !failed_files.empty( ) ) {
        SPDLOG_ERROR( "Failed to restore:" );
        for ( const auto &f : failed_files ) {
            SPDLOG_ERROR( "  - {}", f );
        }
        return false;
    } else {
        SPDLOG_INFO( "backup for: {} has been restored", save_path.filename( ).string( ) );
        return true;
    }
}

void ZipArchive::set_comment( const std::string &str ) {
    zip_set_archive_comment( archive, str.c_str( ), str.size( ) );
}

const char *ZipArchive::get_comment( ) {
    int len = 0;
    return zip_get_archive_comment( archive, &len, 0 );
}

std::string ZipArchive::build_manifest( std::vector<std::pair<fs::path, fs::path>> paths ) {
    json data;
    std::vector<fs::path> failed_files;

    for ( const auto &entry : paths ) {
        auto hash = hash_file( entry.first );
        auto save_time = std::chrono::system_clock::to_time_t( file_time_to_sys( fs::last_write_time( entry.first ) ) );

        if ( hash.empty( ) ) {
            failed_files.emplace_back( entry.second );
            continue;
        }
        data[entry.second.string( )] = { { "hash", hash }, { "mtime", save_time } };
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

bool ZipArchive::write_manifest_to_zip( zip_t *archive ) {
    if ( archive == nullptr ) {
        SPDLOG_ERROR( "invalid archive" );
        return false;
    }

    zip_source_t *source = zip_source_buffer( archive, manifest.data( ), manifest.size( ), 0 );
    if ( source == nullptr ) {
        SPDLOG_ERROR( "Failed to create source for manifest" );
        return false;
    }

    if ( zip_file_add( archive, "manifest.json", source, ZIP_FL_OVERWRITE ) < 0 ) {
        SPDLOG_ERROR( "Failed to add manifest to zip" );
        zip_source_free( source );
        return false;
    }

    return true;
}

bool ZipArchive::read_manifest_from_zip( zip_t *archive ) {
    if ( archive == nullptr ) {
        SPDLOG_ERROR( "invalid archive" );
        return false;
    }

    auto entries = zip_get_num_entries( archive, ZIP_FL_UNCHANGED );
    bool found = false;
    json data;

    for ( size_t i{ }; i < entries; i++ ) {
        std::string found_file = zip_get_name( archive, i, ZIP_FL_UNCHANGED );
        if ( ( found_file.compare( "manifest.json" ) ) != 0 ) continue;

        zip_file *file = zip_fopen_index( archive, i, 0 );
        if ( file == nullptr ) {
            SPDLOG_WARN( "Failed to open manifest in archive" );
            continue;
        }

        struct zip_stat fileInfo;
        zip_stat_init( &fileInfo );
        if ( zip_stat_index( archive, i, 0, &fileInfo ) == 0 ) {
            std::string content( fileInfo.size, '\0' );
            zip_fread( file, content.data( ), fileInfo.size );
            zip_fclose( file );
            data = json::parse( content );
            found = true;
        }
    }

    if ( !found ) SPDLOG_WARN( "manifest.json not found in archive" );
    manifest = data.dump( );
    return true;
}

std::vector<std::string> ZipArchive::get_entry_names( ) {
    std::vector<std::string> entry_names;

    auto entries = zip_get_num_entries( archive, ZIP_FL_UNCHANGED );
    for ( size_t i{ }; i < entries; i++ ) {
        std::string name = zip_get_name( archive, i, ZIP_FL_UNCHANGED );
        if ( ( name.compare( "manifest.json" ) ) == 0 ) continue;

        entry_names.emplace_back( name );
    }
    return entry_names;
}
