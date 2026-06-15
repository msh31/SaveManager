#pragma once

class CTaskRunner {
    public:
        void run( std::function<void( )> work, std::function<void( )> on_complete );
        void update( );

        void shutdown( );

    private:
        struct Task {
                std::future<void> future;
                std::function<void( )> on_complete;
        };

        std::vector<Task> m_tasks;
};
