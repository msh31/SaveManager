#pragma once
#include <branding.hpp>
#include <utils/paths.hpp>

#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#define MSG_LIMIT 500

template <typename Mutex> class ringbuffer_sink : public spdlog::sinks::base_sink<Mutex> {
    public:
        void clear( ) {
            std::lock_guard<Mutex> lock( spdlog::sinks::base_sink<Mutex>::mutex_ );
            messages.clear( );
        }

        std::deque<std::string> get_messages( ) {
            std::lock_guard<Mutex> lock( spdlog::sinks::base_sink<Mutex>::mutex_ );
            return messages;
        }

    private:
        std::deque<std::string> messages;

    protected:
        void sink_it_( const spdlog::details::log_msg& msg ) override {
            spdlog::memory_buf_t formatted;
            spdlog::sinks::base_sink<Mutex>::formatter_->format( msg, formatted );

            if ( messages.size( ) >= MSG_LIMIT ) messages.pop_front( );
            messages.emplace_back( formatted.data( ), formatted.size( ) );
        }
        void flush_( ) override {}
};

using ringbuffer_sink_mt = ringbuffer_sink<std::mutex>;

inline std::shared_ptr<ringbuffer_sink_mt> g_ringbuffer_sink;

inline void setup_logger( ) {
    g_ringbuffer_sink = std::make_shared<ringbuffer_sink_mt>( );
#ifdef SPDLOG_WCHAR_FILENAMES
    spdlog::filename_t log_path = paths::log_file( ).wstring( );
#else
    spdlog::filename_t log_path = paths::log_file( ).string( );
#endif

    std::vector<spdlog::sink_ptr> sinks{ g_ringbuffer_sink };
    sinks.push_back( std::make_shared<spdlog::sinks::stdout_color_sink_mt>( ) ); // TODO: confirm this doesnt make a
                                                                                 // console window pop up - it shouldnt
    sinks.push_back( std::make_shared<spdlog::sinks::daily_file_sink_mt>( log_path, 0, 0 ) );

    auto app_logger = std::make_shared<spdlog::logger>( APP_NAME, sinks.begin( ), sinks.end( ) );
    spdlog::set_default_logger( app_logger );
}

inline ringbuffer_sink_mt* get_ringbuffer_sink( ) { return g_ringbuffer_sink.get( ); }
