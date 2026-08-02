#include "pcgw.hpp"
#include "../detector_utils.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace {
    const std::unordered_map<std::string, SaveRoot> TOKEN_TO_ROOT = {
        { "USER_PROFILE", SaveRoot::USER_PROFILE },
        { "USER_PROFILE_DOCUMENTS", SaveRoot::DOCUMENTS },
        { "APPDATA", SaveRoot::APPDATA },
        { "APPDATA_LOCALLOW", SaveRoot::LOCAL_APPDATA_LOW },
        { "LOCAL_APPDATA", SaveRoot::LOCAL_APPDATA },
        { "PROGRAM_DATA", SaveRoot::PROGRAM_DATA },
        { "PROGRAM_FILES", SaveRoot::PROGRAM_FILES },
        { "OSX_HOME", SaveRoot::OSX_HOME },
        { "LINUX_HOME", SaveRoot::LINUX_HOME },
        { "XDG_DATA_HOME", SaveRoot::XDG_DATA_HOME },
        { "XDG_CONFIG_HOME", SaveRoot::XDG_CONFIG_HOME },
        { "STEAM_DIR", SaveRoot::STEAM_DIR },
    };

    constexpr uint64_t STEAM_ID64_BASE = 76561197960265728ULL;

#if defined( _WIN32 )
    constexpr std::string_view CURRENT_OS = "Windows";
#elif defined( __linux__ )
    constexpr std::string_view CURRENT_OS = "Linux";
#elif defined( __APPLE__ )
    constexpr std::string_view CURRENT_OS = "OS X";
#endif

    constexpr std::string_view WINE_OS = "Windows";

    fs::path resolve_wine_root( SaveRoot sr, const WineRootCtx& wine ) {
        switch ( sr ) {
        case SaveRoot::USER_PROFILE:
            return wine.user_home;
        case SaveRoot::DOCUMENTS:
            return wine.user_home / "Documents";
        case SaveRoot::APPDATA:
            return wine.user_home / "AppData" / "Roaming";
        case SaveRoot::LOCAL_APPDATA:
            return wine.user_home / "AppData" / "Local";
        case SaveRoot::LOCAL_APPDATA_LOW:
            return wine.user_home / "AppData" / "LocalLow";
        case SaveRoot::SAVED_GAMES:
            return wine.user_home / "Saved Games";
        case SaveRoot::PROGRAM_DATA:
            return wine.drive_c / "ProgramData";
        case SaveRoot::PROGRAM_FILES:
            return wine.drive_c / "Program Files";
        case SaveRoot::STEAM_DIR:
            return SteamHelper::get_steam_location( ).value_or( fs::path{ } ).parent_path( ).parent_path( );
        default:
            return { };
        }
    }
} // namespace

CPCGamingWikiDetector::CPCGamingWikiDetector(
    const SteamManifestCache& manifest_cache, const std::unordered_map<uint32_t, std::vector<PcgwEntry>>& entries )
    : m_manifest_cache( manifest_cache ), m_entries( entries ) {}

std::unordered_map<uint32_t, std::vector<PcgwEntry>> CPCGamingWikiDetector::load_manifest( ) {
    std::unordered_map<uint32_t, std::vector<PcgwEntry>> entries;

    std::ifstream file( paths::pcgw_manifest( ) );
    if ( !file.is_open( ) ) {
        SPDLOG_WARN( "[PCGamingWiki] failed to open manifest at: {}", paths::pcgw_manifest( ).string( ) );
        return entries;
    }

    try {
        json data = json::parse( file );
        for ( const auto& entry : data ) {
            if ( !entry.contains( "appid" ) || !entry.contains( "saves" ) ) {
                // #ifndef NDEBUG
                //                 SPDLOG_WARN( "[PCGamingWiki] entry has no appid or saves entry, skipping.." );
                // #endif
                continue;
            }

            std::string page = entry.value( "page", "" );

            uint32_t appid;
            try {
                appid = static_cast<uint32_t>( std::stoul( entry["appid"].get<std::string>( ) ) );
            } catch ( const std::exception& ex ) {
                // #ifndef NDEBUG
                //                 SPDLOG_WARN( "[PCGamingWiki] failed to get appid from entry: {}", ex.what( ) );
                // #endif
                continue;
            }

            for ( const auto& save : entry["saves"] ) {
                if ( !save.value( "clean", false ) ) {
                    // #ifndef NDEBUG
                    //                     SPDLOG_WARN( "[PCGamingWiki] entry is not clean, skipping.." );
                    // #endif
                    continue;
                }

                std::string path = save.value( "path", "" );
                if ( path.empty( ) ) {
                    // #ifndef NDEBUG
                    //                     SPDLOG_WARN( "[PCGamingWiki] {} is empty, skipping..", path );
                    // #endif
                    continue;
                }

                entries[appid].push_back( { save.value( "os", "" ), path, page } );
            }
        }
        SPDLOG_INFO( "[PCGamingWiki] loaded entries for {} games from manifest", entries.size( ) );
    } catch ( const json::exception& ex ) {
        SPDLOG_ERROR( "[PCGamingWiki] failed to parse manifest: {}", ex.what( ) );
    }

    return entries;
}

std::optional<fs::path>
CPCGamingWikiDetector::resolve( const std::string& raw_path, const SteamManifest* manifest, const WineRootCtx* wine ) {
    std::string result = { };
    size_t i = 0;

    while ( i < raw_path.size( ) ) {
        if ( raw_path[i] != '<' ) {
            result += raw_path[i];
            i++;
            continue;
        }

        auto close = raw_path.find( '>', i );
        if ( close == std::string::npos ) return std::nullopt;

        std::string token = raw_path.substr( i + 1, close - i - 1 );
        fs::path resolved;

        if ( token == "GAME_INSTALL_DIR" ) {
            if ( manifest == nullptr ) return { };
            resolved = manifest->library_dir / "steamapps" / "common" / manifest->install_dir;
        } else if ( token == "STEAM_LIBRARY_DIR" ) {
            if ( manifest == nullptr ) return { };
            resolved = manifest->library_dir;
        } else if ( token == "USER_ID" ) {
            auto steamid64 = SteamHelper::parse_steam_userid( );
            if ( !steamid64 ) return std::nullopt;
            try {
                uint64_t account_id = std::stoull( *steamid64 );
                if ( result.ends_with( "userdata\\" ) ) {
                    auto short_id = account_id - STEAM_ID64_BASE;
                    resolved = std::to_string( short_id );
                } else {
                    resolved = std::to_string( account_id );
                }
            } catch ( const std::exception& ) {
                return std::nullopt;
            }
        } else if ( auto it = TOKEN_TO_ROOT.find( token ); it != TOKEN_TO_ROOT.end( ) ) {
            resolved = wine ? resolve_wine_root( it->second, *wine ) : save::resolve_root( it->second );
            if ( resolved.empty( ) ) return std::nullopt;
        } else {
            return std::nullopt;
        }

        result += resolved.string( );
        i = close + 1;
    }

    std::ranges::replace( result, '\\', '/' );
    auto pos = result.rfind( '/' ); // backwards from find

    if ( pos != std::string::npos ) {
        std::string segment = result.substr( pos + 1 );
        auto wildcard_found = segment.find( '*' );
        if ( wildcard_found != std::string::npos ) {
            result = result.substr( 0, pos );
        }
    }

    if ( !result.empty( ) && result.back( ) == '/' ) result.pop_back( );
    return fs::path( result );
}

std::expected<std::vector<Game>, SMError> CPCGamingWikiDetector::find( ) {
    std::vector<Game> games;

    for ( const auto& [appid, manifest] : m_manifest_cache.get_app_manifests( ) ) {
        auto it = m_entries.find( appid );
        if ( it == m_entries.end( ) ) continue;

        for ( const auto& entry : it->second ) {
            if ( entry.os != CURRENT_OS ) continue;

            auto resolved = resolve( entry.raw_path, &manifest, nullptr );
            if ( !resolved || !fs::exists( *resolved ) ) continue;

            Game game;
            game.type = PlatformType::PCGAMINGWIKI;
            game.platform_label = std::string( PLATFORM_LABEL );
            game.game_name = manifest.name;
            game.appid = std::to_string( appid );
            game.save_paths.push_back( *resolved );
            game.show_parent_path = true;

            SPDLOG_INFO( "[PCGamingWiki] found: {}", game.game_name );
            games.push_back( std::move( game ) );
        }
    }

    return games;
}

std::vector<Game> CPCGamingWikiDetector::scan_wine_user( const fs::path& user_home, const DetectorContext& ctx ) {
    std::vector<Game> games = { };

    auto appid = resolve_prefix_appid( user_home );
    if ( !appid ) {
#ifndef NDEBUG
        SPDLOG_WARN( "[PCGamingWiki] Failed to get appid in: {}", user_home.string( ) );
#endif
        return games;
    }

    auto it = ctx.pcgw_entries.find( *appid );
    if ( it == ctx.pcgw_entries.end( ) ) {
#ifndef NDEBUG
        SPDLOG_WARN( "[PCGamingWiki] Failed to find {} in: {}", *appid, user_home.string( ) );
#endif
        return games;
    }

    const auto& manifests = ctx.manifest_cache.get_app_manifests( );
    auto manifest_it = manifests.find( *appid );
    const SteamManifest* manifest = nullptr;
    if ( manifest_it != manifests.end( ) ) {
        manifest = &manifest_it->second;
    }

    WineRootCtx wine{ user_home, user_home.parent_path( ).parent_path( ) };

    for ( const auto& entry : it->second ) {
        if ( entry.os != WINE_OS && entry.os != "Steam" ) continue;

        auto resolved = resolve( entry.raw_path, manifest, &wine );
        if ( !resolved ) {
#ifndef NDEBUG
            SPDLOG_WARN( "[PCGamingWiki] Failed to resolve {}", entry.raw_path );
#endif
            continue;
        }
        if ( !fs::exists( *resolved ) ) {
#ifndef NDEBUG
            SPDLOG_WARN( "[PCGamingWiki] {} does not exist!", entry.raw_path );
#endif
            continue;
        }
        if ( !manifest && entry.page.empty( ) ) continue;

        Game game;
        game.type = PlatformType::PCGAMINGWIKI;
        game.platform_label = std::string( CPCGamingWikiDetector::PLATFORM_LABEL );
        game.game_name = manifest ? manifest->name : entry.page; // big bad ternary but cleaner here
        game.appid = std::to_string( *appid );
        game.save_paths.push_back( *resolved );
        game.show_parent_path = true;

        SPDLOG_INFO( "[PCGamingWiki] found (wine): {}", game.game_name );
        games.push_back( std::move( game ) );
    }

    return games;
}

std::string_view CPCGamingWikiDetector::name( ) const { return PLATFORM_LABEL; }
