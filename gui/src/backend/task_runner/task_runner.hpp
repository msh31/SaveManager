#pragma once

class CTaskRunner {
    public:
        void
        run( std::function<void( )> work, std::function<void( )> on_complete,
             std::function<void( const std::exception& )> on_error );
        void update( );

        void shutdown( );

    private:
        struct Task {
                std::future<void> future;
                std::function<void( )> on_complete;
                std::function<void( const std::exception& )> on_error;
        };

        std::vector<Task> m_tasks;
};
