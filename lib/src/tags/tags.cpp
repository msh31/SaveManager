#include <tags/tags.hpp>
#include <utils/paths.hpp>
#include <utils/utils.hpp>
#include <logger.hpp>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

void Tags::migrate_labels_to_tags( ) {
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

            bool write_res = utils::atomic_write( tags_file, j_tags.dump( 4 ) );
            if ( !write_res ) {
                SPDLOG_ERROR( "[Tags] Failed to migrate tags!" );
            }
            fs::remove( labels_file );
        }
    }
}

std::unordered_map<std::string, std::vector<std::string>> Tags::load_tags( const std::string& game ) {
    std::unordered_map<std::string, std::vector<std::string>> tags;
    std::string file_name = ( paths::backup_dir( ) / utils::sanitize_filename_path( game ) / "tags.json" ).string( );
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

std::expected<bool, SMError> Tags::save_tags( const std::string& game, const std::string& filename, const std::vector<std::string>& tags ) {
    std::string file_name = ( paths::backup_dir( ) / utils::sanitize_filename_path( game ) / "tags.json" ).string( );
    std::string tmp_name = file_name + ".tmp";

    json data = load_tags( game );
    data[filename] = tags;

    if ( data[filename].empty( ) ) {
        data.erase( filename );
    }

    if ( data.empty( ) ) {
        fs::remove( file_name );
        return true;
    }

    bool write_res = utils::atomic_write( tmp_name, data.dump( 4 ) );
    if ( !write_res ) {
        SPDLOG_ERROR( "[Tags] Failed to save tags due to a write error!" );
        return false;
    }

    std::error_code ec;
    fs::rename( tmp_name, file_name, ec );
    if ( ec ) {
        SPDLOG_ERROR( "Failed to move tags into place for: {}", game );
        return false;
    }
    return true;
}

bool Tags::delete_tags( const std::string& game, const std::string& filename ) {
    std::string file_name = ( paths::backup_dir( ) / utils::sanitize_filename_path( game ) / "tags.json" ).string( );

    json data = load_tags( game );
    data.erase( filename );

    if ( data.empty( ) ) {
        fs::remove( file_name );
        return true;
    }

    return utils::atomic_write( file_name, data.dump( 4 ) );
}