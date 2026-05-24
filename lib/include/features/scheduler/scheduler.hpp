#pragma once
#include <config/config.hpp>
#include <types.hpp>

class SaveScheduler {
  public:
    SaveScheduler( const CConfig &config ) {
        m_config = config;
        load( );
    }
    ~SaveScheduler( ) {
        stop( );
        save( );
    }

    void add_entry( ScheduleEntry entry );
    void remove_entry( std::string appid );
    void update_entry( ScheduleEntry entry );

    void run( );
    void save( );

  private:
    CConfig m_config;

    void load( );
    void stop( ) {
        m_running = false;
        m_cv.notify_one( );
        if ( m_backup_thread.joinable( ) ) {
            m_backup_thread.join( );
        }
    }
    void backup_loop( );

    std::vector<ScheduleEntry> m_entries;
    std::mutex schedule_mutex;
    std::thread m_backup_thread;
    std::atomic<bool> m_running{ false };

    std::condition_variable m_cv;
    std::mutex m_stop_mutex;
};
