#include <CLI/CLI.hpp>

#include "config/config.hpp"
#include <detection/detection.hpp>
#include <utils/blacklist/blacklist.hpp>
#include <utils/translations/translations.hpp>

Detection::DetectionResult result;
CConfig                    config;

int main( int argc, char* argv[] ) {
    if ( argc < 2 ) SPDLOG_ERROR( "Use --help to see a full list of available commands!" );

    init_logger( "[%n]: [%l] %d-%m-%Y %H:%M:%S - %v" );
    spdlog::set_level( spdlog::level::err );

    translations::init( );
    Blacklist::init( );
    CLI::App app{ "The CLI version of SaveManager" };
    argv = app.ensure_utf8( argv );

    // commands
    auto list    = app.add_subcommand( "list", "Shows detected games" );
    auto backup  = app.add_subcommand( "backup", "Create a backup" );
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
        Detection::find_saves( config, result );

        for ( const auto& entry : result.games ) {
            std::println( "{}", entry.game_name );
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
