#include <network/network.hpp>
#include <utils/ludisavi_parser/ludusavi_parser.hpp>
#include <utils/steam/steam.hpp>

CLudusaviParser::CLudusaviParser( ) {
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

    m_manifest = YAML::LoadFile( m_path.string( ) );

    for ( auto it = m_manifest.begin( ); it != m_manifest.end( ); ++it ) {
        std::string game_name = it->first.as<std::string>( );
        YAML::Node  game_node = it->second;

        uint32_t game_id = 0;
        if ( game_node["steam"]["id"] ) {
            game_id = game_node["steam"]["id"].as<uint32_t>( );
        } else
            continue;

        m_index.emplace( game_id, game_node );
    }
}

CLudusaviParser::~CLudusaviParser( ) {
    m_manifest_str.clear( );
    m_index.clear( );
}

std::vector<ManifestSavePath> CLudusaviParser::get_save_paths( uint32_t appid ) {
    std::vector<ManifestSavePath> save_paths;

    auto it = m_index.find( appid );
    if ( it == m_index.end( ) ) return { };
    YAML::Node game_node = it->second;

    for ( auto it = game_node["files"].begin( ); it != game_node["files"].end( ); ++it ) {
        ManifestSavePath entry;
        entry.unresolved_path = it->first.as<std::string>( );

        for ( auto tag : it->second["tags"] ) {
            entry.tags.emplace_back( tag.as<std::string>( ) );
        }

        if ( it->second["when"] ) {
            for ( auto cond : it->second["when"] ) {
                if ( cond["os"] ) {
                    std::string os = cond["os"].as<std::string>( );
                    if ( os == "windows" ) entry.is_windows = true; // horrible..
                    if ( os == "linux" ) entry.is_linux = true;
                    if ( os == "mac" ) entry.is_mac = true;
                }
            }
        } else {
            entry.is_windows = entry.is_linux = entry.is_mac = true;
        }

        entry.resolved_path  = resolve_path( entry.unresolved_path.string( ) );
        entry.fully_resolved = !entry.unresolved_path.string( ).contains( "<" );

        if ( !std::ranges::contains( entry.tags, "save" ) ) continue;
        save_paths.emplace_back( entry );
    }

    return save_paths;
}

fs::path CLudusaviParser::resolve_path( std::string_view raw ) {
    auto home     = paths::home_dir( ).string( );
    auto xdg_dh   = std::getenv( "XDG_DATA_HOME" );
    auto xdg_ch   = std::getenv( "XDG_CONFIG_HOME" );
    auto xdg_data = xdg_dh ? xdg_dh : home + "/.local/share";
    auto xdg_cfg  = xdg_ch ? xdg_ch : home + "/.config";
    auto user_id  = SteamHelper::parse_steam_userid( ).value_or( "" );

    std::unordered_map<std::string, std::string> vars = {
        { "<home>", home }, { "<xdgData>", xdg_data }, { "<xdgConfig>", xdg_cfg }, { "<storeUserId>", user_id } };

    std::string result{ raw };
    for ( auto& [var, val] : vars ) {
        auto pos = result.find( var );
        if ( pos != std::string::npos ) result.replace( pos, var.length( ), val );
    }

    return fs::path( result );
}
