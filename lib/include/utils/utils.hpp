#pragma once
#include <types.hpp>

#include <openssl/evp.h>
#include <openssl/sha.h>
#include <zip.h>

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

// apple clang doesnt support c++23 views as of apr 2026
template <typename Range, typename Fn> void enumerate( Range& range, Fn fn ) {
#ifdef __APPLE__
    int i = 0;
    for ( auto& r : range ) {
        fn( i, r );
        ++i;
    }
#else
    for ( auto [i, element] : std::views::enumerate( range ) ) {
        fn( i, element );
    }
#endif
}

// Source - https://stackoverflow.com/a/5253245
// Posted by Blastfurnace, modified by community. See post 'Timeline' for change history
// Retrieved 2026-02-03, License - CC BY-SA 2.5
inline std::string space2underscore( std::string text ) {
    std::replace( text.begin( ), text.end( ), ' ', '_' );
    return text;
}

inline std::string sanitize_filename( std::string text ) {
    const std::string invalid = "<>:\"/\\|?*";
    std::replace_if(
        text.begin( ), text.end( ), [&]( char c ) { return invalid.find( c ) != std::string::npos; }, '_' );
    return text;
}

// i hate windows and special characters
inline fs::path utf8_to_path( const std::string& utf8 ) {
    return fs::path(
        reinterpret_cast<const char8_t*>( utf8.data( ) ),
        reinterpret_cast<const char8_t*>( utf8.data( ) + utf8.size( ) ) );
}

inline std::string path_to_utf8( const fs::path& p ) {
    auto u8 = p.u8string( );
    return std::string( reinterpret_cast<const char*>( u8.data( ) ), u8.size( ) );
}

inline fs::path sanitize_filename_path( const std::string& text ) { return utf8_to_path( sanitize_filename( text ) ); }

// inline fs::path operator/( const fs::path& lhs, const std::string& utf8_rhs ) { return lhs / utf8_to_path( utf8_rhs
// ); }

static std::string_view get_platform_label( PlatformType t ) {
    switch ( t ) {
    case PlatformType::UBISOFT:
        return "Ubisoft";
    case PlatformType::ROCKSTAR:
        return "Rockstar";
    case PlatformType::UNREAL:
        return "Unreal";
    case PlatformType::MINECRAFT:
        return "Minecraft"; // change to launcher?
    case PlatformType::CUSTOM:
        return "CUSTOM";
    case PlatformType::GENERIC:
        return "??";
    }
    return "";
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
    extern char** environ;
    pid_t pid;

    const char* argv[] = { "open", path, nullptr };
    int status = posix_spawn( &pid, "/usr/bin/open", nullptr, nullptr, (char* const*)argv, environ );
    if ( status == 0 ) {
        waitpid( pid, &status, 0 );
    }
#endif
}

// TODO: move everything into this namespace so the code is easier to navigate
namespace utils {
    inline GameKey get_game_identity_key( const Game& game ) {
        if ( !game.appid.empty( ) && game.appid != "N/A" ) return { GameKeyKind::STEAM_APPID, game.appid };

        // ubisoft
        if ( game.game_id.has_value( ) ) return { GameKeyKind::UBISOFT_ID, *game.game_id };

        // minecraft launchers
        if ( game.type == PlatformType::MINECRAFT ) return { GameKeyKind::MINECRAFT, game.game_name };

        if ( !game.game_name.empty( ) ) return { GameKeyKind::NAME, game.game_name };

        if ( !game.save_paths.empty( ) ) {
            SPDLOG_INFO( "save path hit: {}", game.game_name );
            return { GameKeyKind::PATH, weakly_canonical( game.save_paths[0] ).string( ) };
        }

        SPDLOG_ERROR( "Failed to get game identify key" );
        return { GameKeyKind::INVALID }; // caller must check this
    }
} // namespace utils

inline std::vector<std::vector<int>> get_grouped( const std::vector<Game>& games ) {
    std::map<GameKey, size_t> key_to_group;
    std::vector<std::vector<int>> groups;

    enumerate( games, [&]( int i, auto& game ) {
        auto key = utils::get_game_identity_key( game );

        auto it = key_to_group.find( key );
        if ( it != key_to_group.end( ) ) {
            groups[it->second].push_back( i );
        } else {
            key_to_group[key] = groups.size( );
            groups.push_back( { static_cast<int>( i ) } );
        }
    } );

    return groups;
}

// file_clock::to_sys / from_sys are not available on MSVC or Apple Clang
static std::chrono::system_clock::time_point file_time_to_sys( fs::file_time_type ft ) {
    return std::chrono::system_clock::now( ) +
           std::chrono::duration_cast<std::chrono::system_clock::duration>( ft - fs::file_time_type::clock::now( ) );
}

static fs::file_time_type sys_to_file_time( std::chrono::system_clock::time_point tp ) {
    return fs::file_time_type::clock::now( ) +
           std::chrono::duration_cast<fs::file_time_type::clock::duration>( tp - std::chrono::system_clock::now( ) );
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
