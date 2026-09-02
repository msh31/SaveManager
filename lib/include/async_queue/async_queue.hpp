#pragma once

struct TaskCancelled : std::runtime_error {
        TaskCancelled( ) : std::runtime_error( "Task cancelled" ) {}
};

struct TaskControl {
        std::atomic<float> progress{ 0.0f };
        std::atomic<bool> cancel_requested{ false };
};

class TaskHandle {
    public:
        explicit TaskHandle( std::shared_ptr<TaskControl> control ) : m_control( std::move( control ) ) {}

        float progress( ) const { return m_control->progress.load( ); }
        void request_cancel( ) { m_control->cancel_requested.store( true ); }
        bool cancel_requested( ) const { return m_control->cancel_requested.load( ); }

    private:
        std::shared_ptr<TaskControl> m_control;
};

class CAsyncQueue {
    public:
        template <typename T>
        TaskHandle
        run( std::function<T( TaskControl& )> work, std::function<void( T )> on_complete,
             std::function<void( const std::exception& )> on_error ) {
            auto result = std::make_shared<T>( );
            auto control = std::make_shared<TaskControl>( );

            auto wl = [work, result, control]( ) { *result = work( *control ); };
            auto cl = [result, on_complete]( ) { on_complete( *result ); };
            auto el = [on_error]( const std::exception& ex ) { on_error( ex ); };

            m_tasks.emplace_back( Task{ std::async( std::launch::async, wl ), cl, el } );
            return TaskHandle{ control };
        }
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
