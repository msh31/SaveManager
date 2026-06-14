#include <CLI/CLI.hpp>

#include "config/config.hpp"
#include <detection/detection.hpp>
#include <utils/blacklist/blacklist.hpp>
#include <utils/translations/translations.hpp>

CConfig config;

int main( int argc, char* argv[] ) {
    if ( argc < 2 ) SPDLOG_ERROR( "Use --help to see a full list of available commands!" );

    init_logger( "[%n]: [%l] %d-%m-%Y %H:%M:%S - %v" );
    spdlog::set_level( spdlog::level::err );

    translations::init( );
    Blacklist::init( );
    CLI::App app{ "The CLI version of SaveManager" };
    argv = app.ensure_utf8( argv );

    // commands
    auto list = app.add_subcommand( "list", "Shows detected games" );
    auto backup = app.add_subcommand( "backup", "Create a backup" );
    auto restore = app.add_subcommand( "restore", "Restore a backup" );

    // subcommands
    std::string game_id, save_id = { };
    backup->add_option( "game_id", game_id, "Game appid" );
    backup->add_flag( "--all", "Backup all games" );

    std::string backup_id = { };
    restore->add_option( "game_id", game_id, "Game appid" );
    restore->add_option( "backup_id", backup_id, "Game appid" );

    CLI11_PARSE( app, argc, argv );

    if ( *list ) {
        std::vector<Game> result = Detection::find_saves( );
        if ( result.empty( ) ) {
            SPDLOG_CRITICAL( "No Savegames found!" );
            return 1;
        }

        for ( const auto& entry : result ) {
            std::println( "- {} (AppID: {})", entry.game_name, entry.appid );

            for ( const auto& path : entry.save_paths ) {
                for ( const auto& save :
                      fs::recursive_directory_iterator( path, fs::directory_options::skip_permission_denied ) ) {
                    if ( save.is_directory( ) ) continue;
                    std::println( "  - {}", save.path( ).filename( ).string( ) );
                }
            }
        }
    }
    if ( *backup ) {
        SPDLOG_INFO( "BACKUP COMMAND" );
    }
    if ( *restore ) {
        SPDLOG_INFO( "RESTORE COMMAND" );
    }

    return 0;
}
