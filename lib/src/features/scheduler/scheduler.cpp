#include "features/backup/backup.hpp"
#include <features/scheduler/scheduler.hpp>
#include <utils/paths.hpp>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

void CSaveScheduler::load( ) {
    std::ifstream file( paths::schedule_file( ).c_str( ) );
    if ( !file.is_open( ) ) {
        SPDLOG_ERROR( "Failed to open schedule for loading!" );
        return;
    }
    try {
        json data = json::parse( file );

        if ( data.is_array( ) ) {
            for ( const auto& item : data ) {
                ScheduleEntry entry;
                entry.enabled   = item.value( "enabled", true );
                entry.game_name = item.value( "game_name", "" );
                entry.appid     = item.value( "appid", "" );
                // entry.save_path = item.value( "save_path", "" );
                for ( const auto& s : item.value( "included_saves", std::vector<std::string>{ } ) ) {
                    entry.included_saves.push_back( fs::path( s ) );
                }
                entry.type             = item.value( "type", PlatformType::GENERIC );
                entry.interval_hours   = item.value( "interval_hours", 2 );
                entry.last_backup_time = item.value( "last_backup_time", 0 );
                m_entries.push_back( entry );
            }
        }
    } catch ( json::exception& ex ) {
        SPDLOG_ERROR( "schedule parsing error: {}", ex.what( ) );
    }
}

void CSaveScheduler::save( ) {
    json arr = json::array( );
    for ( const auto& entry : m_entries ) {
        json obj;
        obj["enabled"]   = entry.enabled;
        obj["game_name"] = entry.game_name;
        obj["appid"]     = entry.appid;
        obj["save_path"] = entry.save_path;
        json paths_arr   = json::array( );
        for ( const auto& p : entry.included_saves ) {
            paths_arr.push_back( p.string( ) );
        }
        obj["included_saves"]   = paths_arr;
        obj["type"]             = entry.type;
        obj["interval_hours"]   = entry.interval_hours;
        obj["last_backup_time"] = entry.last_backup_time;
        arr.push_back( obj );
    }
    std::ofstream file( paths::schedule_file( ) );
    file << arr.dump( 4 );
}

void CSaveScheduler::backup_loop( ) {
    while ( m_running ) {
        std::vector<ScheduleEntry> to_backup;
        {
            std::lock_guard<std::mutex> lock( m_schedule_mutex );
            auto                        now_ts = std::chrono::duration_cast<std::chrono::seconds>(
                                                     std::chrono::system_clock::now( ).time_since_epoch( ) )
                                                     .count( );

            for ( const auto& entry : m_entries ) {
                if ( entry.enabled && now_ts - entry.last_backup_time > entry.interval_hours * 3600 )
                    to_backup.push_back( entry );
            }
        }

        for ( const auto& entry : to_backup ) {
            Game game;
            game.game_name = entry.game_name;
            game.appid     = entry.appid;
            game.type      = entry.type;
            if ( entry.included_saves.empty( ) ) {
                // game.save_path = entry.save_path; //TODO
                Features::backup_game( game, entry.save_path, m_config );
            } else {
                for ( const auto& save : entry.included_saves ) {
                    // game.save_path = save.string( );
                    Features::backup_game( game, save, m_config );
                }
            }
        }

        if ( !to_backup.empty( ) ) {
            auto                        now_ts = std::chrono::duration_cast<std::chrono::seconds>(
                                                     std::chrono::system_clock::now( ).time_since_epoch( ) )
                                                     .count( );
            std::lock_guard<std::mutex> lock( m_schedule_mutex );
            for ( auto& e : m_entries )
                for ( const auto& b : to_backup )
                    if ( e.appid == b.appid ) e.last_backup_time = now_ts;
            save( );
        }

        std::unique_lock<std::mutex> lock( m_stop_mutex );
#ifndef NDEBUG
        m_cv.wait_for( lock, std::chrono::seconds( 10 ), [this] { return !m_running.load( ); } );
#else
        m_cv.wait_for( lock, std::chrono::minutes( 10 ), [this] { return !m_running.load( ); } );
#endif
    }
}

void CSaveScheduler::run( ) {
    m_running       = true;
    m_backup_thread = std::thread( &CSaveScheduler::backup_loop, this );
}

void CSaveScheduler::add_entry( ScheduleEntry entry ) {
    {
        std::lock_guard<std::mutex> lock( m_schedule_mutex );
        m_entries.emplace_back( entry );
        SPDLOG_INFO( "added new schedule entry for: {}", entry.game_name );
        save( );
    }
}

void CSaveScheduler::remove_entry( std::string appid ) {
    {
        std::lock_guard<std::mutex> lock( m_schedule_mutex );
        SPDLOG_INFO( "removed schedule entry for appid: {}", appid );
        std::erase_if( m_entries, [&]( const auto& e ) { return e.appid == appid; } );
    }
}

void CSaveScheduler::update_entry( ScheduleEntry entry ) {
    {
        std::lock_guard<std::mutex> lock( m_schedule_mutex );
        SPDLOG_INFO( "updating schedule entry for: {}", entry.game_name );
        auto it = std::ranges::find_if( m_entries, [&]( const auto& e ) { return e.appid == entry.appid; } );
        if ( it != m_entries.end( ) ) *it = entry;
    }
}
