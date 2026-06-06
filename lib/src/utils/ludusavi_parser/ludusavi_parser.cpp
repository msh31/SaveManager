#include <network/network.hpp>
#include <utils/ludisavi_parser/ludusavi_parser.hpp>
#include <utils/steam/steam.hpp>

#include <nlohmann/json.hpp>
#include <yaml-cpp/yaml.h>

using json = nlohmann::json;

namespace {
    bool is_cache_fresh( const fs::path& cache_path, const fs::path& manifest_path ) {
        if ( !fs::exists( cache_path ) || !fs::exists( manifest_path ) ) return false;
        return fs::last_write_time( cache_path ) >= fs::last_write_time( manifest_path );
    }

    std::optional<std::unordered_map<uint32_t, std::vector<ManifestSavePath>>>
    load_index_cache( const fs::path& cache_path ) {
        if ( !fs::exists( cache_path ) ) return std::nullopt;

        try {
            std::ifstream input( cache_path );
            json          data = json::parse( input );

            std::unordered_map<uint32_t, std::vector<ManifestSavePath>> index;

            for ( auto it = data.begin( ); it != data.end( ); ++it ) {
                uint32_t                      appid = std::stoul( it.key( ) );
                std::vector<ManifestSavePath> entries;

                for ( const auto& item : it.value( ) ) {
                    ManifestSavePath entry;
                    entry.unresolved_path = item.value( "path", "" );
                    entry.is_windows      = item.value( "is_windows", false );
                    entry.is_linux        = item.value( "is_linux", false );
                    entry.is_mac          = item.value( "is_mac", false );
                    for ( const auto& tag : item.value( "tags", json::array( ) ) ) {
                        entry.tags.emplace_back( tag.get<std::string>( ) );
                    }
                    entries.emplace_back( std::move( entry ) );
                }

                if ( !entries.empty( ) ) index.emplace( appid, std::move( entries ) );
            }

            return index;
        } catch ( const std::exception& ex ) {
            SPDLOG_WARN( "Failed to load Ludusavi cache: {}", ex.what( ) );
            return std::nullopt;
        }
    }

    void save_index_cache(
        const fs::path& cache_path, const std::unordered_map<uint32_t, std::vector<ManifestSavePath>>& index ) {
        try {
            fs::create_directories( cache_path.parent_path( ) );
            json data = json::object( );
            for ( const auto& [appid, entries] : index ) {
                auto& arr = data[std::to_string( appid )];
                arr       = json::array( );
                for ( const auto& entry : entries ) {
                    arr.push_back(
                        { { "path", entry.unresolved_path.string( ) },
                          { "is_windows", entry.is_windows },
                          { "is_linux", entry.is_linux },
                          { "is_mac", entry.is_mac },
                          { "tags", entry.tags } } );
                }
            }

            std::ofstream output( cache_path );
            output << data.dump( );
        } catch ( const std::exception& ex ) {
            SPDLOG_WARN( "Failed to write Ludusavi cache: {}", ex.what( ) );
        }
    }
} // namespace

CLudusaviParser::CLudusaviParser( ) {
    m_home               = paths::home_dir( ).string( );
    const char* xdg_data = std::getenv( "XDG_DATA_HOME" );
    const char* xdg_cfg  = std::getenv( "XDG_CONFIG_HOME" );
    m_xdg_data           = xdg_data ? xdg_data : m_home + "/.local/share";
    m_xdg_config         = xdg_cfg ? xdg_cfg : m_home + "/.config";
    m_store_user_id      = SteamHelper::parse_steam_userid( ).value_or( "" );

#if defined( _WIN32 )
    const char* user = std::getenv( "USERNAME" );
#else
    const char* user = std::getenv( "USER" );
#endif
    m_os_user_name = user ? user : "";

    if ( m_manifest_exists ) {
        auto ftime = fs::last_write_time( m_path );
        if ( fs::file_time_type::clock::now( ) - ftime > std::chrono::weeks( 1 ) ) {
            m_is_outdated = true;
        }
    }

    // perhaps make this optional
    if ( !m_manifest_exists || m_is_outdated ) {
        if ( !Network::download_file( m_manifest_link, m_path.string( ) ) ) {
            SPDLOG_ERROR( "Failed to download manifest!" );
            return;
        }
    }

    if ( is_cache_fresh( m_index_cache_path, m_path ) ) {
        if ( auto cached = load_index_cache( m_index_cache_path ) ) {
            m_index = std::move( *cached );
            return;
        }
    }

    YAML::Node manifest = YAML::LoadFile( m_path.string( ) );

    for ( auto it = manifest.begin( ); it != manifest.end( ); ++it ) {
        YAML::Node game_node = it->second;

        if ( !game_node["steam"] || !game_node["steam"]["id"] ) continue;
        uint32_t game_id = game_node["steam"]["id"].as<uint32_t>( );

        if ( !game_node["files"] ) continue;

        std::vector<ManifestSavePath> entries;
        for ( auto fit = game_node["files"].begin( ); fit != game_node["files"].end( ); ++fit ) {
            ManifestSavePath entry{ };
            entry.unresolved_path = fit->first.as<std::string>( );

            for ( auto tag : fit->second["tags"] ) {
                entry.tags.emplace_back( tag.as<std::string>( ) );
            }

            if ( !std::ranges::contains( entry.tags, "save" ) ) continue;

            if ( fit->second["when"] ) {
                for ( auto cond : fit->second["when"] ) {
                    if ( cond["os"] ) {
                        std::string os = cond["os"].as<std::string>( );
                        if ( os == "windows" ) entry.is_windows = true;
                        if ( os == "linux" ) entry.is_linux = true;
                        if ( os == "mac" ) entry.is_mac = true;
                    }
                }
            } else {
                entry.is_windows = entry.is_linux = entry.is_mac = true;
            }

            entries.emplace_back( std::move( entry ) );
        }

        if ( !entries.empty( ) ) m_index.emplace( game_id, std::move( entries ) );
    }

    save_index_cache( m_index_cache_path, m_index );
}

CLudusaviParser::~CLudusaviParser( ) { m_index.clear( ); }

std::vector<ManifestSavePath> CLudusaviParser::get_save_paths( uint32_t appid, const ResolveContext& ctx ) {
    auto it = m_index.find( appid );
    if ( it == m_index.end( ) ) return { };

    std::vector<ManifestSavePath> save_paths = it->second;

    for ( auto& entry : save_paths ) {
        entry.resolved_path  = resolve_path( entry.unresolved_path.string( ), entry, ctx, appid );
        entry.fully_resolved = !entry.resolved_path.string( ).contains( "<" );
    }

    std::erase_if( save_paths, []( const ManifestSavePath& e ) { return !e.fully_resolved; } );

    return save_paths;
}

fs::path CLudusaviParser::resolve_path(
    std::string_view raw, const ManifestSavePath& entry, const ResolveContext& ctx, uint32_t appid ) const {
    std::unordered_map<std::string, std::string> vars;

    const std::string base = ctx.install_dir.string( );
    vars["<root>"]         = base;
    vars["<base>"]         = base;
    vars["<game>"]         = base;
    vars["<storeUserId>"]  = m_store_user_id;
    vars["<storeGameId>"]  = std::to_string( appid );

    vars["<xdgData>"]   = m_xdg_data;
    vars["<xdgConfig>"] = m_xdg_config;

#if defined( _WIN32 )
    (void)entry;
    const std::string home    = m_home;
    vars["<home>"]            = home;
    vars["<osUserName>"]      = m_os_user_name;
    vars["<winAppData>"]      = ( paths::home_dir( ) / "AppData" / "Roaming" ).string( );
    vars["<winLocalAppData>"] = ( paths::home_dir( ) / "AppData" / "Local" ).string( );
    vars["<winDocuments>"]    = paths::documents_dir( ).string( );
    vars["<winPublic>"]       = "C:\\Users\\Public";
    vars["<winProgramData>"]  = "C:\\ProgramData";
    vars["<winDir>"]          = "C:\\Windows";
#else
    const bool use_prefix = entry.is_windows && !ctx.proton_prefix.empty( );
    if ( use_prefix ) {
        const fs::path drive_c   = ctx.proton_prefix / "drive_c";
        const fs::path steamuser = drive_c / "users" / "steamuser";

        vars["<home>"]            = steamuser.string( );
        vars["<osUserName>"]      = "steamuser";
        vars["<winAppData>"]      = ( steamuser / "AppData" / "Roaming" ).string( );
        vars["<winLocalAppData>"] = ( steamuser / "AppData" / "Local" ).string( );
        vars["<winDocuments>"]    = ( steamuser / "Documents" ).string( );
        vars["<winPublic>"]       = ( drive_c / "users" / "Public" ).string( );
        vars["<winProgramData>"]  = ( drive_c / "ProgramData" ).string( );
        vars["<winDir>"]          = ( drive_c / "windows" ).string( );
    } else {
        vars["<home>"]       = m_home;
        vars["<osUserName>"] = m_os_user_name;
    }
#endif

    std::string result{ raw };
    for ( const auto& [var, val] : vars ) {
        size_t pos = 0;
        while ( ( pos = result.find( var, pos ) ) != std::string::npos ) {
            result.replace( pos, var.length( ), val );
            pos += val.length( );
        }
    }

    return fs::path( result );
}
