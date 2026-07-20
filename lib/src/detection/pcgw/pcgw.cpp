#include "pcgw.hpp"

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
            return SteamHelper::get_steam_location( ).value_or( fs::path{ } );
        default:
            return { };
        }
    }
} // namespace

CPCGamingWikiDetector::CPCGamingWikiDetector( const SteamManifestCache& manifest_cache )
    : m_manifest_cache( manifest_cache ), m_entries( load_manifest( ) ) {}

std::unordered_map<uint32_t, std::vector<PcgwEntry>> CPCGamingWikiDetector::load_manifest( ) {
    std::unordered_map<uint32_t, std::vector<PcgwEntry>> entries;

    std::ifstream file( paths::pcgw_manifest( ) );
    if ( !file.is_open( ) ) {
        SPDLOG_WARN( "[PCGamingWiki] no manifest found at {}", paths::pcgw_manifest( ).string( ) );
        return entries;
    }

    try {
        json data = json::parse( file );
        for ( const auto& entry : data ) {
            if ( !entry.contains( "appid" ) || !entry.contains( "saves" ) ) continue;

            uint32_t appid;
            try {
                appid = static_cast<uint32_t>( std::stoul( entry["appid"].get<std::string>( ) ) );
            } catch ( const std::exception& ) {
                continue;
            }

            for ( const auto& save : entry["saves"] ) {
                if ( !save.value( "clean", false ) ) continue;

                std::string path = save.value( "path", "" );
                if ( path.empty( ) ) continue;

                entries[appid].push_back( { save.value( "os", "" ), path } );
            }
        }
        SPDLOG_INFO( "[PCGamingWiki] loaded entries for {} games from manifest", entries.size( ) );
    } catch ( const json::exception& ex ) {
        SPDLOG_ERROR( "[PCGamingWiki] failed to parse manifest: {}", ex.what( ) );
    }

    return entries;
}

std::optional<fs::path>
CPCGamingWikiDetector::resolve( const std::string& raw_path, const SteamManifest& manifest, const WineRootCtx* wine ) {
    std::string result;
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
            resolved = manifest.library_dir / "steamapps" / "common" / manifest.install_dir;
        } else if ( token == "STEAM_LIBRARY_DIR" ) {
            resolved = manifest.library_dir;
        } else if ( token == "USER_ID" ) {
            auto steamid64 = SteamHelper::parse_steam_userid( );
            if ( !steamid64 ) return std::nullopt;
            try {
                uint64_t account_id = std::stoull( *steamid64 ) - STEAM_ID64_BASE;
                resolved = std::to_string( account_id );
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
    return fs::path( result );
}

std::expected<std::vector<Game>, SMError> CPCGamingWikiDetector::find( ) {
    std::vector<Game> games;

    for ( const auto& [appid, manifest] : m_manifest_cache.get_app_manifests( ) ) {
        auto it = m_entries.find( appid );
        if ( it == m_entries.end( ) ) continue;

        for ( const auto& entry : it->second ) {
            if ( entry.os != CURRENT_OS ) continue;

            auto resolved = resolve( entry.raw_path, manifest, nullptr );
            if ( !resolved || !fs::exists( *resolved ) ) continue;

            Game game;
            game.type = PlatformType::PCGAMINGWIKI;
            game.platform_label = std::string( PLATFORM_LABEL );
            game.game_name = manifest.name;
            game.appid = std::to_string( appid );
            game.save_paths.push_back( *resolved );

            SPDLOG_INFO( "[PCGamingWiki] found: {}", game.game_name );
            games.push_back( std::move( game ) );
        }
    }

    return games;
}

std::vector<Game> CPCGamingWikiDetector::scan_wine_user( const fs::path& user_home, const DetectorContext& ctx ) {
    static const auto entries = load_manifest( );

    std::vector<Game> games;
    WineRootCtx wine{ user_home, user_home.parent_path( ).parent_path( ) };

    for ( const auto& [appid, manifest] : ctx.manifest_cache.get_app_manifests( ) ) {
        auto it = entries.find( appid );
        if ( it == entries.end( ) ) continue;

        for ( const auto& entry : it->second ) {
            if ( entry.os != WINE_OS ) continue;

            auto resolved = resolve( entry.raw_path, manifest, &wine );
            if ( !resolved || !fs::exists( *resolved ) ) continue;

            Game game;
            game.type = PlatformType::PCGAMINGWIKI;
            game.platform_label = std::string( CPCGamingWikiDetector::PLATFORM_LABEL );
            game.game_name = manifest.name;
            game.appid = std::to_string( appid );
            game.save_paths.push_back( *resolved );

            SPDLOG_INFO( "[PCGamingWiki] found (wine): {}", game.game_name );
            games.push_back( std::move( game ) );
        }
    }

    return games;
}

std::string_view CPCGamingWikiDetector::name( ) const { return PLATFORM_LABEL; }
