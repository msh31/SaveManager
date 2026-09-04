#include <tags/tags.hpp>
#include <utils/paths.hpp>
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