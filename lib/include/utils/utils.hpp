#pragma once
#include <logger.hpp>
#include <SHA256.h>

#ifdef __APPLE__
    #include <ctime>
    #include <spawn.h>
    #include <sys/wait.h>
#elif __linux__
    #include <sys/wait.h>
#endif
#ifdef _WIN32
    #include <shellapi.h>
#endif

#ifdef __APPLE__
extern char** environ;
#endif

namespace utils { // All functions in this namespace should work across Windows, Linux and macOS
    inline std::string get_username( ) {
        const char* usrname = nullptr;

#if defined( __linux__ ) || defined( __APPLE__ )
        usrname = std::getenv( "USER" );
#else
        usrname = std::getenv( "USERNAME" );
#endif
        if ( !usrname ) throw std::runtime_error( "USER not set, how did you manage to do this?" );
        return std::string( usrname );
    }

    inline void open_in_file_manager( const char* path ) {
#ifdef __linux__
        pid_t pid = fork( );
        pid_t w = 0;
        int status;

        if ( pid > 0 ) {
            w = waitpid( pid, &status, 0 );
            if ( w == -1 ) {
                SPDLOG_ERROR( "waitpid failed: {}", strerror( errno ) );
            }
        }

        if ( pid == 0 ) {
            pid_t g_pid = fork( );

            if ( g_pid == 0 ) {
                execl( "/usr/bin/xdg-open", "xdg-open", path, nullptr );
                _exit( 1 );
            }
            _exit( 0 );
        }
#endif
#ifdef _WIN32
        ShellExecuteA( NULL, "open", path, NULL, NULL, SW_SHOWDEFAULT );
#endif
#ifdef __APPLE__
        pid_t pid;

        const char* argv[] = { "open", path, nullptr };
        int status = posix_spawn( &pid, "/usr/bin/open", nullptr, nullptr, (char* const*)argv, environ );
        if ( status == 0 ) {
            waitpid( pid, &status, 0 );
        }
#endif
    }

    // file_clock::to_sys / from_sys are not available on MSVC or Apple Clang
    static std::chrono::system_clock::time_point file_time_to_sys( fs::file_time_type ft ) {
        return std::chrono::system_clock::now( ) + std::chrono::duration_cast<std::chrono::system_clock::duration>(
                                                       ft - fs::file_time_type::clock::now( ) );
    }

    static fs::file_time_type sys_to_file_time( std::chrono::system_clock::time_point tp ) {
        return fs::file_time_type::clock::now( ) + std::chrono::duration_cast<fs::file_time_type::clock::duration>(
                                                       tp - std::chrono::system_clock::now( ) );
    }

    static std::string format_file_time( fs::file_time_type f ) {
#ifdef __APPLE__
        char buf[32];
        auto ts = std::chrono::system_clock::to_time_t( file_time_to_sys( f ) );
        auto tm = std::localtime( &ts );
        std::strftime( buf, sizeof( buf ), "%d-%m-%y %H:%M:%S", tm );
        return buf;
#else
        auto time = std::chrono::current_zone( )->to_local( file_time_to_sys( f ) );
        auto floored = std::chrono::floor<std::chrono::seconds>( time );
        return std::format( "{:%d-%m-%y %H:%M:%S}", floored );
#endif
    }

    static std::string format_file_size( uintmax_t size ) {
        constexpr uintmax_t KB = 1024, MB = KB * 1024, GB = MB * 1024;

        if ( size >= GB ) return std::format( "{:.2f}GB", static_cast<double>( size ) / GB );
        if ( size >= MB ) return std::format( "{:.2f}MB", static_cast<double>( size ) / MB );
        if ( size >= KB ) return std::format( "{:.2f}KB", static_cast<double>( size ) / KB );
        return std::format( "{}B", size );
    }

    // simply writes the contents to a tmp file and flushes the contents onto the drive
    static bool atomic_write( const fs::path& path, const std::string& content ) {
        if ( fs::is_directory( path ) ) return false;

        fs::path tmp_path = path;
        tmp_path += ".tmp";

        std::ofstream file( tmp_path, std::ios::binary );
        if ( !file.is_open( ) ) {
            SPDLOG_ERROR( "[AtomicWrite]: failed to open temp file for writing!" );
            return false;
        }

        auto cleanup = [&file]( fs::path cpath ) {
            file.close( );
            std::error_code ecr;
            fs::remove( cpath, ecr );
            if ( ecr ) SPDLOG_ERROR( "[AtomicWrite]: {}", ecr.message( ) );
        };

        file.write( content.data( ), content.size( ) );
        if ( !file.good( ) ) {
            SPDLOG_ERROR( "[AtomicWrite]: failed to write to temp file!" );
            cleanup( tmp_path );
            return false;
        }

        if ( !file.flush( ) ) {
            SPDLOG_ERROR( "[AtomicWrite]: failed to flush content to disk!" );
            cleanup( tmp_path );
            return false;
        }

        file.close( );

        std::error_code ec;
        fs::rename( tmp_path, path, ec );

        if ( ec ) {
            SPDLOG_ERROR( "[AtomicWrite]: rename error {}", ec.message( ) );
            cleanup( tmp_path );
            return false;
        }

        return true;
    }

    static std::string hash_file( const fs::path& path ) {
        if ( !fs::is_regular_file( path ) ) return { };

        std::ifstream file( path, std::ios::binary );
        if ( !file.is_open( ) ) return { };

        SHA256 sha;
        char buffer[8192];
        while ( file.read( buffer, sizeof( buffer ) ) ) {
            sha.update( reinterpret_cast<uint8_t*>( buffer ), file.gcount( ) );
        }

        if ( file.gcount( ) > 0 ) sha.update( reinterpret_cast<uint8_t*>( buffer ), file.gcount( ) );
        file.close( );

        std::array<uint8_t, 32> digest = sha.digest( );
        return SHA256::toString( digest );
    }
} // namespace utils