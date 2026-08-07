#pragma once

class CTaskRunner {
    public:
        template <typename T>
        void
        run( std::function<T( )> work, std::function<void( T )> on_complete,
             std::function<void( const std::exception& )> on_error ) {
            auto result = std::make_shared<T>( );

            auto wl = [work, result]( ) { *result = work( ); };
            auto cl = [result, on_complete]( ) { on_complete( *result ); };
            auto el = [on_error]( const std::exception& ex ) { on_error( ex ); };

            m_tasks.emplace_back( Task{ std::async( std::launch::async, wl ), cl, el } );
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
