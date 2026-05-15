#pragma once 
#include <types.hpp>
#include <config/config.hpp>

class SaveScheduler {
    public:
        SaveScheduler(const Config& config) {
            load();
        }
        ~SaveScheduler() {
            stop();
            save();
        }

        void add_entry(ScheduleEntry entry);
        void remove_entry(std::string appid);
        void update_entry(ScheduleEntry entry);
        
        void run();
        void save();
    private:
        Config config;

        void load();
        void stop() {
            m_running = false;
            if(m_backup_thread.joinable()) {
                m_backup_thread.join();
            }
        }
        void backup_loop();

        std::vector<ScheduleEntry> m_entries;
        std::mutex schedule_mutex;
        std::thread m_backup_thread;
        std::atomic<bool> m_running;

        ScheduleEntry schedule;
};
