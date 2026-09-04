#include <backup/backup.hpp>
#include <utils/utils.hpp>
#include "../utils/zip_archive/zip_archive.hpp"
#include <logger.hpp>

bool Backup::backup_game( const Game& game, const fs::path& file, CConfig& config ) {
    SPDLOG_INFO( "creating backup of: {}", game.game_name );
    fs::path game_backup_dir = paths::backup_dir( ) / utils::sanitize_filename_path( game.game_name );

    auto ext = file.extension( ).string( );
    if ( ( game.type == PlatformType::MINECRAFT || game.type == PlatformType::GENERIC ) &&
         !fs::is_regular_file( file ) ) {
        if ( !fs::is_directory( file ) ) return false;
    } else {
        if ( !fs::is_regular_file( file ) || extension_blocklist.contains( ext ) ) {
            return false;
        }
    }

    if ( !fs::exists( game_backup_dir ) ) fs::create_directories( game_backup_dir );

    fs::path final_path = game_backup_dir / utils::utf8_to_path( construct_backup_name( game.game_name ) );
    fs::path zip_name = final_path.parent_path( ) / ( utils::path_to_utf8( final_path.filename( ) ) + ".tmp" );

    // writing happens on the destructor so we scope it to do it immediatly, needs a refactor
    bool success = false;
    {
        CZipArchive archive( MODE_CREATE_ARCHIVE, zip_name.u8string( ) );
        if ( game.type == PlatformType::MINECRAFT ) {
            archive.set_comment( utils::path_to_utf8( file ) );
        } else {
            archive.set_comment( utils::path_to_utf8( file.parent_path( ) ) );
        }
        success = archive.add_to_archive( file ) && archive.finalize_add( );
    }

    if ( !success ) {
        fs::remove( zip_name );
        SPDLOG_ERROR( "failed to create backup for: {}", game.game_name );
        return false;
    }
    std::error_code ec;
    fs::rename( zip_name, final_path, ec );
    if ( ec ) {
        SPDLOG_ERROR( "rename failed: {}", ec.message( ) );
        return false;
    }
    SPDLOG_INFO( "backup created: {}", game.game_name );
    return true;
}

std::vector<std::string> Backup::backup_all_games( const std::vector<Game>& snapshot, CConfig& config ) {
    std::vector<std::string> failures = { };

    for ( const auto& entry : snapshot ) {
        std::vector<std::pair<fs::path, const Game*>> files;

        for ( const auto& save : entry.save_paths ) {
            if ( !fs::is_directory( save ) ) continue;
            for ( const auto& file :
                  fs::recursive_directory_iterator( save, fs::directory_options::skip_permission_denied ) ) {
                if ( !fs::is_regular_file( file ) ) continue;

                auto ext = file.path( ).extension( ).string( );
                if ( extension_blocklist.contains( ext ) ) continue;

                files.push_back( { file.path( ), &entry } );
            }
        }
        if ( files.empty( ) ) continue;

        if ( !backup_game_files( entry, files ) ) {
            SPDLOG_WARN( "Failed to create backup for: {}", entry.game_name );
            failures.emplace_back( entry.game_name );
            continue;
        }
    }
    return failures;
}

bool Backup::backup_to_path( fs::path source, fs::path dest ) {
    if ( !fs::exists( dest.parent_path( ) ) ) fs::create_directories( dest.parent_path( ) );

    fs::path zip_name = fs::path( dest.string( ) + ".tmp" );
    bool success = false;
    {
        CZipArchive archive( MODE_CREATE_ARCHIVE, zip_name.u8string( ) );
        std::string original_path = source.parent_path( ).string( );
        if ( fs::is_directory( source ) ) {
            original_path = source.string( );
        }
        archive.set_comment( original_path );
        success = archive.add_to_archive( source ) && archive.finalize_add( );
    }

    if ( !success ) {
        fs::remove( zip_name );
        SPDLOG_ERROR( "failed to create undo backup" );
        return false;
    } else {
        std::error_code ec;
        fs::rename( zip_name, dest, ec );
        if ( ec ) {
            SPDLOG_ERROR( "undo rename failed: {}", ec.message( ) );
            return false;
        }
    }
    SPDLOG_INFO( "undo backup created" );
    return true;
}

bool Backup::backup_game_files( const Game& game, std::vector<std::pair<fs::path, const Game*>> files ) {
    fs::path game_backup_dir = paths::backup_dir( ) / utils::sanitize_filename_path( game.game_name );
    if ( !fs::exists( game_backup_dir ) ) fs::create_directories( game_backup_dir );

    fs::path final_path = game_backup_dir / utils::utf8_to_path( construct_backup_name( game.game_name ) );
    fs::path zip_name = final_path.parent_path( ) / ( utils::path_to_utf8( final_path.filename( ) ) + ".tmp" );

    bool failed_to_add = false;
    {
        CZipArchive za( MODE_CREATE_ARCHIVE, zip_name );
        for ( const auto& entry : files ) {
            for ( size_t i = { }; i < game.save_paths.size( ); i++ ) {
                fs::path result = fs::relative( entry.first, game.save_paths[i] );
                if ( !result.string( ).starts_with( ".." ) ) {
                    fs::path entry_path =
                        ( game.save_paths.size( ) > 1 ) ? fs::path( std::to_string( i ) ) / result : result;
                    if ( !za.add_to_archive( entry.first, std::nullopt, entry_path.string( ) ) ) failed_to_add = true;
                    break;
                }
            }
        }
        if ( !za.finalize_add( ) ) {
            failed_to_add = true;
        }
    }

    if ( failed_to_add ) {
        std::error_code ec;
        fs::remove( zip_name, ec );
        if ( ec ) {
            SPDLOG_ERROR( "[Backup] failed to remove temporary zip after add failure, manual cleanup is advised!" );
        }
        return false;
    }

    std::error_code ec;
    fs::rename( zip_name, final_path, ec );
    if ( ec ) {
        SPDLOG_ERROR( "[Backup] rename failed: {}", ec.message( ) );
        return false;
    }

    return true;
}

bool Backup::restore_backup(
    const fs::path& name, const std::vector<fs::path>& save_paths,
    std::vector<std::pair<fs::path, fs::path>>& conflicts, std::unordered_set<std::string> exclusions ) {
    CZipArchive archive( MODE_EXTRACT_ARCHIVE, name.u8string( ) );

    std::string comment = archive.get_comment( );
    if ( !comment.empty( ) ) {
        fs::path restore_path;
        restore_path = comment;
        auto entries = archive.get_entry_names( );
        fs::path undo_source = ( entries.size( ) == 1 ) ? restore_path / entries[0] : restore_path;
        fs::create_directories( restore_path );

        if ( fs::exists( undo_source ) && !fs::is_empty( undo_source ) ) {
            if ( !backup_to_path( undo_source, name.parent_path( ) / "undo.zip" ) ) {
                SPDLOG_ERROR( "Failed to create backup before overwriting newer savefile, aborting.." );
                return false;
            }
        }
    }
    bool has_index_prefixes = comment.empty( ) ? ( save_paths.size( ) > 1 ) : false;
    if ( !archive.extract_archive( save_paths, conflicts, has_index_prefixes, exclusions ) ) {
        SPDLOG_ERROR( "failed to restore backup: {}", name.filename( ).string( ) );
        return false;
    }
    SPDLOG_INFO( "backup restored: {}", name.filename( ).string( ) );
    return true;
}

std::string Backup::construct_backup_name( const std::string& game, const std::string& custom_name ) {
    auto now = std::chrono::system_clock::now( );
    auto timestamp = std::format( "{:%Y%m%d_%H%M%S}", now );

    std::string game_name = utils::sanitize_filename( game );
    std::string game_name_sanitized = utils::space2underscore( game_name );
    std::string filename = custom_name;

    if ( filename.empty( ) ) {
        filename = game_name_sanitized;
    }
    return std::format( "backup_{}_{}.zip", filename, timestamp );
}

// caller must do empty check
std::vector<std::string> Backup::get_backup_entries( const fs::path& backup ) {
    CZipArchive archive( MODE_EXTRACT_ARCHIVE, backup.u8string( ) );
    std::vector<std::string> entries;
    entries = archive.get_entry_names( );
    return entries;
}