#include "features/features.hpp"
#include "../utils/zip_archive/zip_archive.hpp"
#include "config/config.hpp"
#include "constants.hpp"
#include "types.hpp"
#include "utils/utils.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

bool Features::backup_game( const Game& game, const fs::path& file, CConfig& config ) {
    SPDLOG_INFO( "creating backup of: {}", game.game_name );
    fs::path game_backup_dir = paths::backup_dir( ) / sanitize_filename( game.game_name );

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

    fs::path final_path = game_backup_dir / construct_backup_name( game.game_name );
    fs::path zip_name = final_path.parent_path( ) / ( final_path.filename( ).string( ) + ".tmp" );

    // writing happens on the destructor so we scope it to do it immediatly, needs a refactor
    bool success = false;
    {
        CZipArchive archive( MODE_CREATE_ARCHIVE, zip_name.u8string( ) );
        if ( game.type == PlatformType::MINECRAFT ) {
            archive.set_comment( file.string( ) );
        } else {
            archive.set_comment( file.parent_path( ).string( ) );
        }
        success = archive.add_to_archive( file ) && archive.finalize_add( );
    }

    if ( !success ) {
        fs::remove( zip_name );
        SPDLOG_ERROR( "failed to create backup for: {}", game.game_name );
        return false;
    } else {
        std::error_code ec;
        fs::rename( zip_name, final_path, ec );
        if ( ec ) {
            SPDLOG_ERROR( "rename failed: {}", ec.message( ) );
            return false;
        }
    }
    SPDLOG_INFO( "backup created: {}", game.game_name );
    return true;
}

std::vector<std::string> Features::backup_all_games( const std::vector<Game>& snapshot, CConfig& config ) {
    std::vector<std::string> failures = { };

    for ( const auto& entry : snapshot ) {
        for ( const auto& save : entry.save_paths ) {
            if ( !fs::is_directory( save ) ) continue;

            for ( const auto& file :
                  fs::recursive_directory_iterator( save, fs::directory_options::skip_permission_denied ) ) {
                if ( !fs::is_regular_file( file ) ) continue;

                auto ext = file.path( ).extension( ).string( );
                if ( extension_blocklist.contains( ext ) ) continue;
                if ( !backup_game( entry, file.path( ), config ) ) {
                    SPDLOG_WARN( "Failed to create backup for: {}", entry.game_name );
                    failures.emplace_back( entry.game_name );
                    continue;
                }
            }
        }
    }
    return failures;
}

bool Features::backup_game_files( const Game& game, std::vector<std::pair<fs::path, const Game*>> files ) {
    fs::path game_backup_dir = paths::backup_dir( ) / sanitize_filename( game.game_name );

    if ( !fs::exists( game_backup_dir ) ) fs::create_directories( game_backup_dir );

    fs::path final_path = game_backup_dir / Features::construct_backup_name( game.game_name );
    fs::path zip_name = final_path.parent_path( ) / ( final_path.filename( ).string( ) + ".tmp" );

    bool failed_to_add = false;
    {
        CZipArchive za( MODE_CREATE_ARCHIVE, zip_name );
        for ( const auto& entry : files ) {
            if ( !za.add_to_archive( entry.first ) ) failed_to_add = true;
        }
        if ( !za.finalize_add( ) ) {
            fs::remove( zip_name );
            return false;
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

bool Features::backup_to_path( fs::path source, fs::path dest ) {
    if ( !fs::exists( dest.parent_path( ) ) ) fs::create_directories( dest.parent_path( ) );

    fs::path zip_name = fs::path( dest.string( ) + ".tmp" );
    bool success = false;
    {
        CZipArchive archive( MODE_CREATE_ARCHIVE, zip_name.u8string( ) );
        archive.set_comment( source.parent_path( ).string( ) );
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

bool Features::restore_backup(
    const fs::path& name, const fs::path& save_path, std::vector<std::pair<fs::path, fs::path>>& conflicts ) {
    CZipArchive archive( MODE_EXTRACT_ARCHIVE, name.u8string( ) );

    fs::path restore_path;
    std::string comment = archive.get_comment( );
    if ( !comment.empty( ) ) {
        restore_path = comment;
        fs::create_directories( restore_path );
    } else {
        restore_path = save_path;
    }

    auto entries = archive.get_entry_names( );
    fs::path undo_source = ( entries.size( ) == 1 ) ? restore_path / entries[0] : restore_path;

    if ( fs::exists( undo_source ) && !fs::is_empty( undo_source ) ) {
        if ( !backup_to_path( undo_source, name.parent_path( ) / "undo.zip" ) ) {
            SPDLOG_ERROR( "Failed to create backup before overwriting newer savefile, aborting.." );
            return false;
        }
    }

    if ( !archive.extract_archive( restore_path, conflicts ) ) {
        SPDLOG_ERROR( "failed to restore backup: {}", name.filename( ).string( ) );
        return false;
    }
    SPDLOG_INFO( "backup restored: {}", name.filename( ).string( ) );
    return true;
}

std::string Features::construct_backup_name( const std::string& game, const std::string& custom_name ) {
    auto now = std::chrono::system_clock::now( );
    auto timestamp = std::format( "{:%Y%m%d_%H%M%S}", now );

    std::string game_name = sanitize_filename( game );
    std::string game_name_sanitized = space2underscore( game_name );
    std::string filename = custom_name;

    if ( filename.empty( ) ) {
        filename = game_name_sanitized;
    }
    return std::format( "backup_{}_{}.zip", filename, timestamp );
}

void Features::migrate_labels_to_tags( ) {
    for ( const auto& dir : fs::directory_iterator( paths::backup_dir( ) ) ) {
        if ( !dir.is_directory( ) ) continue;

        auto labels_file = dir.path( ) / "labels.json";
        auto tags_file = dir.path( ) / "tags.json";

        if ( fs::exists( labels_file ) && !fs::exists( tags_file ) ) {
            // load that mutaphuckin labels file once
            json data;

            std::unordered_map<std::string, std::string> backup_labels; // filename, label
            std::ifstream file( labels_file.c_str( ) );

            if ( !file.is_open( ) ) {
                SPDLOG_CRITICAL( "Label migration: Failed to open labels for {}!", dir.path( ).string( ) );
                continue;
            }

            try {
                data = json::parse( file );
                if ( data.empty( ) ) continue;
                for ( const auto& entry : data.items( ) ) {
                    backup_labels[entry.key( )] = entry.value( ).get<std::string>( );
                }
            } catch ( json::exception& ex ) {
                SPDLOG_ERROR( "Label migration: label parsing error: {}", ex.what( ) );
                continue;
            }

            if ( backup_labels.empty( ) ) continue;
            json j_tags;
            for ( const auto& label : backup_labels ) {
                j_tags[label.first] = json::array( { label.second } );
            }

            std::ofstream out( tags_file );
            if ( !out.is_open( ) ) {
                SPDLOG_ERROR( "Label migration: Failed to write labels.json for: {}", dir.path( ).string( ) );
                continue;
            }

            out << j_tags.dump( 4 );
            if ( !out.good( ) ) {
                out.close( );
                SPDLOG_ERROR( "Failed to save label for: {}", dir.path( ).string( ) );
                continue;
            }
            out.close( );

            fs::remove( labels_file );
        }
    }
}

std::unordered_map<std::string, std::vector<std::string>> Features::load_tags( const std::string& game ) {
    std::unordered_map<std::string, std::vector<std::string>> tags;
    std::string file_name = ( paths::backup_dir( ) / sanitize_filename( game ) / "tags.json" ).string( );
    if ( !fs::exists( file_name ) ) return { };

    std::ifstream in( file_name );
    if ( !in.is_open( ) ) {
        SPDLOG_ERROR( "Failed to load tags for {}!", game );
        return { };
    }

    json data;
    try {
        data = json::parse( in );
        for ( const auto& entry : data.items( ) ) {
            tags[entry.key( )] = entry.value( ).get<std::vector<std::string>>( );
        }
    } catch ( json::exception& ex ) {
        SPDLOG_ERROR( "tag parsing error: {}", ex.what( ) );
        return { };
    }

    return tags;
}

std::expected<bool, SMError>
Features::save_tags( const std::string& game, const std::string& filename, const std::vector<std::string>& tags ) {
    std::string file_name = ( paths::backup_dir( ) / sanitize_filename( game ) / "tags.json" ).string( );

    json data = load_tags( game );
    data[filename] = tags;

    if ( data[filename].empty( ) ) {
        data.erase( filename );
    }

    std::ofstream out( file_name );
    if ( !out.is_open( ) ) {
        SPDLOG_ERROR( "Failed to save tags for: {}", game );
        return false;
    }
    if ( data.empty( ) ) {
        fs::remove( file_name );
        return true;
    }

    out << data.dump( 4 );
    if ( !out.good( ) ) {
        SPDLOG_ERROR( "Failed to save tag for: {}", game );
        return false;
    }
    out.close( );
    return true;
}

bool Features::delete_tags( const std::string& game, const std::string& filename ) {
    std::string file_name = ( paths::backup_dir( ) / sanitize_filename( game ) / "tags.json" ).string( );

    json data = load_tags( game );
    data.erase( filename );

    if ( data.empty( ) ) {
        fs::remove( file_name );
        return true;
    }

    std::ofstream out( file_name );
    if ( !out.is_open( ) ) {
        SPDLOG_ERROR( "Failed to save tags for: {}", game );
        return false;
    }

    out << data.dump( 4 );
    if ( !out.good( ) ) {
        SPDLOG_ERROR( "Failed to save tag for: {}", game );
        return false;
    }
    out.close( );
    return true;
}
