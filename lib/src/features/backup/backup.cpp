#include "features/backup/backup.hpp"
#include "config/config.hpp"
#include "constants.hpp"
#include "types.hpp"
#include "utils/utils.hpp"
#include "utils/zip_archive/zip_archive.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

void Features::backup_game( const Game& game, const fs::path& file, CConfig& config ) {
    SPDLOG_INFO( "creating backup of: {}", game.game_name );
    fs::path game_backup_dir = paths::backup_dir( ) / sanitize_filename( game.game_name );

    auto ext = file.extension( ).string( );
    if ( ( game.type == PlatformType::MINECRAFT || game.type == PlatformType::GENERIC ) &&
         !fs::is_regular_file( file ) ) {
        if ( !fs::is_directory( file ) ) return;
    } else {
        if ( !fs::is_regular_file( file ) ||
             std::find( extension_blocklist.begin( ), extension_blocklist.end( ), ext ) !=
                 extension_blocklist.end( ) ) {
            return;
        }
    }

    if ( !fs::exists( game_backup_dir ) ) fs::create_directories( game_backup_dir );

    fs::path final_path = game_backup_dir / construct_backup_name( game.game_name );
    fs::path zip_name   = final_path.parent_path( ) / ( final_path.filename( ).string( ) + ".tmp" );

    // writing happens on the destructor so we scope it to do it immediatly, needs a refactor
    bool success = false;
    {
        CZipArchive archive( MODE_CREATE_ARCHIVE, zip_name.u8string( ) );
        if ( game.type == PlatformType::MINECRAFT ) {
            archive.set_comment( file.string( ) );
        } else {
            archive.set_comment( file.parent_path( ).string( ) );
        }
        success = archive.add_to_archive( file );
    }

    if ( !success ) {
        fs::remove( zip_name );
        SPDLOG_ERROR( "failed to create backup for: {}", game.game_name );
    } else {
        std::error_code ec;
        fs::rename( zip_name, final_path, ec );
        if ( ec ) SPDLOG_ERROR( "rename failed: {}", ec.message( ) );

        SPDLOG_INFO( "backup created: {}", game.game_name );
    }
}

void Features::backup_all_games( const std::vector<Game>& snapshot, CConfig& config ) {
    for ( const auto& entry : snapshot ) {
        for ( const auto& save : entry.save_paths ) {
            if ( fs::is_directory( save ) ) {
                for ( const auto& file :
                      fs::recursive_directory_iterator( save, fs::directory_options::skip_permission_denied ) ) {
                    if ( fs::is_regular_file( file ) ) {
                        auto ext = file.path( ).extension( ).string( );
                        if ( std::find( extension_blocklist.begin( ), extension_blocklist.end( ), ext ) !=
                             extension_blocklist.end( ) )
                            continue;
                        backup_game( entry, file.path( ), config );
                    }
                }
            }
        }
    }
}

bool Features::backup_game_files( const Game& game, std::vector<std::pair<fs::path, const Game*>> files ) {
    fs::path game_backup_dir = paths::backup_dir( ) / sanitize_filename( game.game_name );

    if ( !fs::exists( game_backup_dir ) ) fs::create_directories( game_backup_dir );

    fs::path final_path = game_backup_dir / Features::construct_backup_name( game.game_name );
    fs::path zip_name   = final_path.parent_path( ) / ( final_path.filename( ).string( ) + ".tmp" );

    bool failed_to_add = false;
    {
        CZipArchive za( MODE_CREATE_ARCHIVE, zip_name );
        for ( const auto& entry : files ) {
            if ( !za.add_to_archive( entry.first ) ) failed_to_add = true;
        }
    }

    if ( failed_to_add ) {
        fs::remove( zip_name );
        return false;
    }

    std::error_code ec;
    fs::rename( zip_name, final_path, ec );
    if ( ec ) SPDLOG_ERROR( "rename failed: {}", ec.message( ) );
    return true;
}

void Features::backup_to_path( fs::path source, fs::path dest ) {
    if ( !fs::exists( dest.parent_path( ) ) ) fs::create_directories( dest.parent_path( ) );

    fs::path zip_name = fs::path( dest.string( ) + ".tmp" );
    bool     success  = false;
    {
        CZipArchive archive( MODE_CREATE_ARCHIVE, zip_name.u8string( ) );
        archive.set_comment( source.parent_path( ).string( ) );
        success = archive.add_to_archive( source );
    }

    if ( !success ) {
        fs::remove( zip_name );
        SPDLOG_ERROR( "failed to create undo backup" );
    } else {
        std::error_code ec;
        fs::rename( zip_name, dest, ec );
        if ( ec ) SPDLOG_ERROR( "undo rename failed: {}", ec.message( ) );

        SPDLOG_INFO( "undo backup created" );
    }
}

std::vector<fs::path> Features::get_backups( const std::string& game ) {
    fs::path game_backup_dir = paths::backup_dir( ) / sanitize_filename( game );

    if ( !fs::exists( game_backup_dir ) ) {
        return { };
    }

    auto backups =
        fs::recursive_directory_iterator( game_backup_dir ) |
        std::views::filter( []( const auto& e ) { return e.is_regular_file( ) && e.path( ).extension( ) == ".zip"; } ) |
        std::views::transform( &fs::directory_entry::path ) | std::ranges::to<std::vector>( );

    return backups;
}

void Features::restore_backup(
    const fs::path& name, const fs::path& save_path, std::vector<std::pair<fs::path, fs::path>>& conflicts ) {
    CZipArchive archive( MODE_EXTRACT_ARCHIVE, name.u8string( ) );

    fs::path    restore_path;
    std::string comment = archive.get_comment( );
    if ( !comment.empty( ) ) {
        restore_path = comment;
        fs::create_directories( restore_path );
    } else {
        restore_path = save_path;
    }

    auto     entries     = archive.get_entry_names( );
    fs::path undo_source = ( entries.size( ) == 1 ) ? restore_path / entries[0] : restore_path;

    if ( fs::exists( undo_source ) && !fs::is_empty( undo_source ) ) {
        backup_to_path( undo_source, name.parent_path( ) / "undo.zip" );
    }

    if ( !archive.extract_archive( restore_path, conflicts ) ) {
        SPDLOG_ERROR( "failed to restore backup: {}", name.filename( ).string( ) );
    } else {
        SPDLOG_INFO( "backup restored: {}", name.filename( ).string( ) );
    }
}

std::string Features::construct_backup_name( const std::string& game, const std::string& custom_name ) {
    auto now       = std::chrono::system_clock::now( );
    auto timestamp = std::format( "{:%Y%m%d_%H%M%S}", now );

    std::string game_name           = sanitize_filename( game );
    std::string game_name_sanitized = space2underscore( game_name );
    std::string filename            = custom_name;

    if ( filename.empty( ) ) {
        filename = game_name_sanitized;
    }
    return std::format( "backup_{}_{}.zip", filename, timestamp );
}

std::unordered_map<std::string, std::string> Features::load_labels( const std::string& game ) {
    json data;

    std::unordered_map<std::string, std::string> backup_labels;
    std::string   file_name = ( paths::backup_dir( ) / sanitize_filename( game ) / "labels.json" ).string( );
    std::ifstream file( file_name.c_str( ) );

    if ( !fs::exists( file_name ) ) return { };

    if ( file.is_open( ) ) {
        try {
            data = json::parse( file );
            for ( const auto& entry : data.items( ) ) {
                backup_labels[entry.key( )] = entry.value( ).get<std::string>( );
            }
        } catch ( json::exception& ex ) {
            SPDLOG_ERROR( "label parsing error: {}", ex.what( ) );
        }

        return backup_labels;
    } else {
        SPDLOG_ERROR( "Failed to open labels to load it!" );
    }

    return { };
}

void Features::save_label( const std::string& game, const std::string& filename, const std::string& label ) {
    std::string file_name = ( paths::backup_dir( ) / sanitize_filename( game ) / "labels.json" ).string( );
    auto        labels    = load_labels( game );

    labels[filename] = label;

    json data;
    for ( const auto& [key, value] : labels ) {
        data[key] = value;
    }

    std::ofstream out( file_name );
    out << data.dump( 4 );
}

void Features::save_labels( const std::string& game, const std::unordered_map<std::string, std::string>& labels ) {
    std::string file_name = ( paths::backup_dir( ) / sanitize_filename( game ) / "labels.json" ).string( );

    json data;
    for ( const auto& [key, value] : labels ) {
        data[key] = value;
    }

    if ( data.empty( ) ) {
        fs::remove( file_name );
    } else {
        std::ofstream out( file_name );
        out << data.dump( 4 );
    }
}
