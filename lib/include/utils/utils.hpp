#pragma once
#include <logger/logger.hpp>
#include <types.hpp>

#include <openssl/evp.h>
#include <openssl/sha.h>
#include <zip.h>

#ifdef __APPLE__
    #include <spawn.h>
    #include <sys/wait.h>
#endif
#ifdef _WIN32
    #include <shellapi.h>
#endif

struct ImFont;

struct Fonts {
        ImFont* regular;
        ImFont* medium;
        ImFont* small_font;
        ImFont* bold;

        ImFont* title;
        ImFont* header;
};

// TODO: refactor transfer tab so this isnt needed
struct TabState {
        std::vector<std::filesystem::path> backups;
        std::vector<bool>                  selected_backups;
        int                                selected_game_idx   = 0;
        int                                selected_backup_idx = 0;
};

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

static std::string_view get_platform_label( PlatformType t ) {
    switch ( t ) {
    case PlatformType::UBISOFT:
        return "Ubisoft";
    case PlatformType::ROCKSTAR:
        return "Rockstar";
    case PlatformType::UNREAL:
        return "Unreal";
    case PlatformType::PSP:
        return "PSP";
    case PlatformType::PPSSPP:
        return "PPSSPP";
    case PlatformType::MINECRAFT:
        return "Minecraft"; // change to launcher?
    case PlatformType::CUSTOM:
        return "CUSTOM";
    case PlatformType::GENERIC:
        return "??";
    }
    return "";
}

static std::string_view get_launcher_label( LauncherType t ) {
    switch ( t ) {
    case LauncherType::OFFICIAL:
        return "Official";
    case LauncherType::MODRINTH:
        return "Modrinth";
    case LauncherType::CURSEFORGE:
        return "CurseForge";
    case LauncherType::PRISM:
        return "Prism";
    case LauncherType::MULTIMC:
        return "MultiMC";
    }
    return "";
}

inline std::string cache_key( const Game& game ) {
    if ( game.type == PlatformType::MINECRAFT ) {
        return "Minecraft";
    }
    return game.game_name;
}

inline std::string hash_file( const std::filesystem::path& path ) {
    if ( !std::filesystem::is_regular_file( path ) ) return { };

    EVP_MD_CTX* mdctx = EVP_MD_CTX_new( );
    if ( mdctx == nullptr ) {
        EVP_MD_CTX_free( mdctx );
        return { };
    }

    const EVP_MD* md = EVP_get_digestbyname( "SHA-256" );
    if ( md == nullptr ) {
        EVP_MD_CTX_free( mdctx );
        return { };
    }

    if ( !EVP_DigestInit_ex( mdctx, md, NULL ) ) {
        EVP_MD_CTX_free( mdctx );
        return { };
    }

    unsigned char hash[SHA256_DIGEST_LENGTH];
    unsigned int  hash_len = 0;
    char          buffer[8192];

    std::ifstream file( path, std::ios::binary );
    if ( !file.is_open( ) ) {
        EVP_MD_CTX_free( mdctx );
        return { };
    }

    while ( file.read( buffer, sizeof( buffer ) ) ) {
        EVP_DigestUpdate( mdctx, buffer, file.gcount( ) );
    }
    if ( file.gcount( ) > 0 ) {
        EVP_DigestUpdate( mdctx, buffer, file.gcount( ) );
    }
    file.close( );

    if ( !EVP_DigestFinal_ex( mdctx, hash, &hash_len ) ) {
        EVP_MD_CTX_free( mdctx );
        return { };
    }
    EVP_MD_CTX_free( mdctx );

    std::stringstream ss;
    for ( int i = 0; i < SHA256_DIGEST_LENGTH; i++ ) {
        ss << std::hex << std::setw( 2 ) << std::setfill( '0' ) << static_cast<int>( hash[i] );
    }
    mdctx = nullptr;
    return ss.str( );
}

inline void open_in_file_manager( const char* path ) {
#ifdef __linux__
    pid_t pid = fork( );
    if ( pid == 0 ) {
        execl( "/usr/bin/xdg-open", "xdg-open", path, nullptr );
        _exit( 1 );
    }
#endif
#ifdef _WIN32
    ShellExecuteA( NULL, "open", path, NULL, NULL, SW_SHOWDEFAULT );
#endif
#ifdef __APPLE__
    extern char** environ;
    pid_t         pid;

    const char* argv[] = { "open", path, nullptr };
    int         status = posix_spawn( &pid, "/usr/bin/open", nullptr, nullptr, (char* const*)argv, environ );
    if ( status == 0 ) {
        waitpid( pid, &status, 0 );
    }
#endif
}
