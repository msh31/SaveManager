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
    constexpr std::string_view CURRENT_OS = "windows";
#elif defined( __linux__ )
    constexpr std::string_view CURRENT_OS = "linux";
#elif defined( __APPLE__ )
    constexpr std::string_view CURRENT_OS = "mac";
#endif

    constexpr std::string_view WINE_OS = "windows";

    const std::unordered_set<std::string> GENERIC_SHARED_ROOT_SEGMENTS = {
        "Documents", "Library", "Application Support", "Desktop", "Roaming",
    };

    bool has_unresolvable_wildcard_segment( const std::string& path ) {
        size_t i = 0;
        while ( i < path.size( ) ) {
            auto open = path.find( '<', i );
            if ( open == std::string::npos ) break;

            auto close = path.find( '>', open );
            if ( close == std::string::npos ) break;

            std::string token = path.substr( open + 1, close - open - 1 );
            if ( TOKEN_TO_ROOT.contains( token ) ) {
                size_t seg_start = close + 1;
                if ( seg_start < path.size( ) && path[seg_start] == '/' ) seg_start++;

                auto seg_end = path.find( '/', seg_start );
                std::string segment =
                    path.substr( seg_start, seg_end == std::string::npos ? std::string::npos : seg_end - seg_start );

                if ( segment.find( '*' ) != std::string::npos ) return true;
            }

            i = close + 1;
        }
        return false;
    }

    bool wildcard_match( std::string_view pattern, std::string_view text ) {
        size_t p = 0, t = 0, star = std::string::npos, match = 0;
        while ( t < text.size( ) ) {
            if ( p < pattern.size( ) && ( pattern[p] == '?' || pattern[p] == text[t] ) ) {
                p++;
                t++;
            } else if ( p < pattern.size( ) && pattern[p] == '*' ) {
                star = p++;
                match = t;
            } else if ( star != std::string::npos ) {
                p = star + 1;
                t = ++match;
            } else {
                return false;
            }
        }
        while ( p < pattern.size( ) && pattern[p] == '*' )
            p++;
        return p == pattern.size( );
    }

    bool is_bare_root_token_path( const std::string& path ) {
        if ( path.empty( ) || path[0] != '<' ) return false;

        auto close = path.find( '>' );
        if ( close == std::string::npos ) return false;

        std::string token = path.substr( 1, close - 1 );
        if ( !TOKEN_TO_ROOT.contains( token ) ) return false;

        std::string rest = path.substr( close + 1 );
        return rest.empty( ) || rest == "/";
    }

    bool is_generic_shared_root_path( const std::string& path ) {
        if ( path.empty( ) || path[0] != '<' ) return false;

        auto close = path.find( '>' );
        if ( close == std::string::npos ) return false;

        std::string token = path.substr( 1, close - 1 );
        if ( !TOKEN_TO_ROOT.contains( token ) ) return false;

        std::string rest = path.substr( close + 1 );
        if ( !rest.empty( ) && rest.front( ) == '/' ) rest.erase( 0, 1 );
        while ( !rest.empty( ) && rest.back( ) == '/' )
            rest.pop_back( );

        if ( rest.empty( ) ) return false;

        auto is_generic_segment = []( const std::string& segment ) {
            return std::ranges::any_of( GENERIC_SHARED_ROOT_SEGMENTS, [&segment]( const std::string& generic ) {
                return segment.size( ) == generic.size( ) &&
                       std::ranges::equal( segment, generic, []( unsigned char a, unsigned char b ) {
                           return std::tolower( a ) == std::tolower( b );
                       } );
            } );
        };

        size_t seg_start = 0;
        while ( seg_start <= rest.size( ) ) {
            auto seg_end = rest.find( '/', seg_start );
            std::string segment =
                rest.substr( seg_start, seg_end == std::string::npos ? std::string::npos : seg_end - seg_start );

            if ( !is_generic_segment( segment ) ) return false;

            if ( seg_end == std::string::npos ) break;
            seg_start = seg_end + 1;
        }

        return true;
    }

    // for things like old windows folders like local settings /applciationdata, my documents etc..
    fs::path resolve_modern_or_legacy_token( const fs::path& root, const fs::path& modern, const fs::path& legacy ) {
        if ( fs::exists( root / modern ) ) return root / modern;
        if ( fs::exists( root / legacy ) ) return root / legacy;
        return root / modern;
    }

    fs::path resolve_wine_root( SaveRoot sr, const WineRootCtx& wine ) {
        switch ( sr ) {
        case SaveRoot::USER_PROFILE:
            return wine.user_home;
        case SaveRoot::DOCUMENTS: {
            fs::path modern = "Documents";
            fs::path legacy = "My Documents";
            return resolve_modern_or_legacy_token( wine.user_home, modern, legacy );
        }
        case SaveRoot::APPDATA: {
            fs::path modern = "AppData/Roaming";
            fs::path legacy = "Application Data";
            return resolve_modern_or_legacy_token( wine.user_home, modern, legacy );
        }
        case SaveRoot::LOCAL_APPDATA: {
            fs::path modern = "AppData/Local";
            fs::path legacy = "Local Settings/Application Data";
            return resolve_modern_or_legacy_token( wine.user_home, modern, legacy );
        }
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

        // if linux ones somehow make it through
        case SaveRoot::LINUX_HOME:
            return save::resolve_root( SaveRoot::LINUX_HOME );
        case SaveRoot::XDG_CONFIG_HOME:
            return save::resolve_root( SaveRoot::XDG_CONFIG_HOME );
        case SaveRoot::XDG_DATA_HOME:
            return save::resolve_root( SaveRoot::XDG_DATA_HOME );
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
                // if ( !save.value( "clean", false ) ) {
                //     // #ifndef NDEBUG
                //     //                     SPDLOG_WARN( "[PCGamingWiki] entry is not clean, skipping.." );
                //     // #endif
                //     continue;
                // }

                std::string path = save.value( "path", "" );
                std::ranges::replace( path, '\\', '/' );

                if ( path.empty( ) ) {
                    // #ifndef NDEBUG
                    //                     SPDLOG_WARN( "[PCGamingWiki] {} is empty, skipping..", path );
                    // #endif
                    continue;
                }

                if ( has_unresolvable_wildcard_segment( path ) || is_bare_root_token_path( path ) ||
                     is_generic_shared_root_path( path ) ) {
                    // #ifndef NDEBUG
                    //                     SPDLOG_WARN( "[PCGamingWiki] {} is a bad manifest entry, skipping..", path
                    //                     );
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

std::vector<Game> CPCGamingWikiDetector::collect_games(
    uint32_t appid, const std::vector<PcgwEntry>& entries, const SteamManifestCache& manifest_cache,
    std::string_view os_filter, const WineRootCtx* wine ) {

    std::vector<Game> games = { };

    const auto& manifests = manifest_cache.get_app_manifests( );
    auto manifest_it = manifests.find( appid );
    const SteamManifest* manifest = nullptr;
    if ( manifest_it != manifests.end( ) ) {
        manifest = &manifest_it->second;
    }

    for ( const auto& entry : entries ) {
        if ( entry.os != os_filter && entry.os != "Steam" ) continue;

        auto resolved_path = resolve( entry.raw_path, manifest, wine );
        if ( !resolved_path.has_value( ) ) continue;
        if ( !fs::exists( resolved_path.value( ) ) ) continue;

        Game game;
        game.type = PlatformType::PCGAMINGWIKI;
        game.platform_label = std::string( PLATFORM_LABEL );
        game.game_name = manifest ? manifest->name : entry.page;
        game.appid = std::to_string( appid );
        game.save_paths.push_back( *resolved_path );
        game.show_parent_path = true;

        SPDLOG_INFO( "[PCGamingWiki] found{}: {}", wine ? " (wine)" : "", game.game_name );
        games.emplace_back( std::move( game ) );
    }

    return games;
}

std::optional<fs::path>
CPCGamingWikiDetector::resolve( std::string raw_path, const SteamManifest* manifest, const WineRootCtx* wine ) {
    bool prefer_short_id = false;
    bool is_user_id_mid_filename = false;
    std::string mid_filename_dir = { };
    std::string mid_filename_pattern = { };
    std::string user_id_parent_dir;
    std::string final_path = { };
    size_t pass_count = 0;

    std::ranges::replace( raw_path, '\\', '/' );

    while ( pass_count < 2 ) {
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
                is_user_id_mid_filename = close + 1 < raw_path.size( ) && raw_path[close + 1] != '/';

                if ( !is_user_id_mid_filename ) {
                    user_id_parent_dir = result;
                }

                mid_filename_dir = result.substr( 0, result.rfind( '/' ) );
                mid_filename_pattern = raw_path.substr( close + 1 );

                final_path = raw_path.substr( close + 1 );
                if ( final_path.starts_with( '/' ) ) final_path = final_path.substr( 1 );

                auto steamid64 = SteamHelper::parse_steam_userid( );
                if ( !steamid64 ) return std::nullopt;

                try {
                    uint64_t account_id = std::stoull( *steamid64 );
                    if ( pass_count == 0 ) {
                        if ( result.ends_with( "userdata/" ) ) {
                            auto short_id = account_id - STEAM_ID64_BASE;
                            resolved = std::to_string( short_id );
                            prefer_short_id = true;
                        } else {
                            resolved = std::to_string( account_id );
                        }
                    }
                    if ( pass_count == 1 ) {
                        if ( !prefer_short_id ) {
                            auto short_id = account_id - STEAM_ID64_BASE;
                            resolved = std::to_string( short_id );
                        } else {
                            resolved = std::to_string( account_id );
                        }
                    }

                } catch ( const std::exception& ) {
                    return std::nullopt;
                }
            } else if ( auto it = TOKEN_TO_ROOT.find( token ); it != TOKEN_TO_ROOT.end( ) ) {
                resolved = wine ? resolve_wine_root( it->second, *wine ) : save::resolve_root( it->second );
                if ( resolved.empty( ) ) {
                    // #ifndef NDEBUG
                    //                     SPDLOG_ERROR( "[PCGamingWIki]: failed to resolve token: {}", token );
                    // #endif
                    return std::nullopt;
                }
            } else {
                return std::nullopt;
            }

            result += resolved.string( );
            i = close + 1;
        }
        pass_count += 1;

        auto pos = result.rfind( '/' ); // backwards from find

        std::string wildcard_pattern = { };
        bool has_wildcard = false;

        if ( pos != std::string::npos ) {
            std::string segment = result.substr( pos + 1 );
            if ( segment.find( '*' ) != std::string::npos || segment.find( '?' ) != std::string::npos ) {
                wildcard_pattern = segment;
                has_wildcard = true;
                result = result.substr( 0, pos );
            }
        }
        if ( !result.empty( ) && result.back( ) == '/' ) result.pop_back( );

        if ( has_wildcard ) {
            if ( fs::exists( result ) && fs::is_directory( result ) ) {
                for ( const auto& entry :
                      fs::directory_iterator( result, fs::directory_options::skip_permission_denied ) ) {
                    if ( wildcard_match( wildcard_pattern, entry.path( ).filename( ).string( ) ) )
                        return fs::path( result );
                }
            }
            // #ifndef NDEBUG
            //             SPDLOG_WARN( "{} has no entries matching {} on the system!", result, wildcard_pattern );
            // #endif
        } else if ( fs::exists( result ) ) {
            return fs::path( result );
        }
        // #ifndef NDEBUG
        //         else
        //             SPDLOG_WARN( "{} does not exist on the system!", result );
        // #endif
    }

    bool user_id_parent_exists = fs::exists( user_id_parent_dir );
    if ( !is_user_id_mid_filename && user_id_parent_exists ) {
        // SPDLOG_INFO( "[PCGamingWiki] using USER_ID fallback: {}", user_id_parent_dir );
        std::vector<fs::path> candidates = { };

        for ( const auto& entry :
              fs::directory_iterator( user_id_parent_dir, fs::directory_options::skip_permission_denied ) ) {

            if ( entry.is_directory( ) ) candidates.push_back( entry.path( ) );
        }

        if ( candidates.empty( ) ) return { };

        for ( const auto& c : candidates ) {
            fs::path p = c / final_path;
            if ( fs::exists( p ) ) return p;
        }
    }
    if ( is_user_id_mid_filename && fs::exists( mid_filename_dir ) ) {
        for ( const auto& entry :
              fs::directory_iterator( mid_filename_dir, fs::directory_options::skip_permission_denied ) ) {
            if ( entry.path( ).filename( ).string( ).ends_with( mid_filename_pattern ) ) {
                return mid_filename_dir;
            }
        }
    }

    // #ifndef NDEBUG
    //     SPDLOG_ERROR(
    //         "[PCGamingWiki] Failed to find: {} | Possible reasons could be that: \n1. Is the user_id in the
    //         middle of a " "filename? {}\n2. Does the user_id_parent_dir exist? {}", raw_path,
    //         is_user_id_mid_filename, user_id_parent_exists );
    // #endif
    return std::nullopt;
}

std::expected<std::vector<Game>, SMError> CPCGamingWikiDetector::find( ) {
    std::vector<Game> games = { };

    for ( const auto& [appid, entries] : m_entries ) {
        auto found = collect_games( appid, entries, m_manifest_cache, CURRENT_OS, nullptr );
        games.insert( games.end( ), found.begin( ), found.end( ) );
    }

    return games;
}

std::vector<Game> CPCGamingWikiDetector::scan_wine_user( const fs::path& user_home, const DetectorContext& ctx ) {
    auto appid = resolve_prefix_appid( user_home );
    if ( !appid ) return { };

    auto it = ctx.pcgw_entries.find( *appid );
    if ( it == ctx.pcgw_entries.end( ) ) return { };

    WineRootCtx wine{ user_home, user_home.parent_path( ).parent_path( ) };

    return collect_games( *appid, it->second, ctx.manifest_cache, WINE_OS, &wine );
}

std::string_view CPCGamingWikiDetector::name( ) const { return PLATFORM_LABEL; }
