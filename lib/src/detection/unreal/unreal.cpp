#include "unreal.hpp"
#include "utils/translations/translations.hpp"

std::string_view CUnrealDetector::name( ) const { return "Unreal"; };

std::expected<std::vector<Game>, SMError> CUnrealDetector::find( ) {
    std::vector<fs::path> prefixes = { };
#ifdef _WIN32
    prefixes.emplace_back( paths::home_dir( ) ); // wow, horrible. no wonder it works like shit on here
#endif
#ifdef __linux__
// todo
#endif
#ifdef __APPLE__
    prefixes.emplace_back( paths::home_dir( ) / "Library" / "Application Support" );
#endif

    std::vector<Game> games;

    for ( const auto& prefix : prefixes ) {
        if ( !fs::exists( prefix ) ) continue;
        SPDLOG_INFO( "[Unreal] searching prefix: {}", prefix.string( ) );

        auto found_games = scan( prefix, m_translations );
        std::ranges::move( found_games, std::back_inserter( games ) );
    }

    return games;
}

// private
std::vector<Game> CUnrealDetector::scan( fs::path path, const Translations& translations ) {
    std::vector<Game> games;

    try {
        if ( !fs::exists( path ) ) return { };
        for ( const auto& folder : fs::directory_iterator( path, fs::directory_options::skip_permission_denied ) ) {
            if ( !fs::is_directory( folder ) ) continue;

            auto target = resolve_save_games( folder );
            if ( !target.has_value( ) ) {
                for ( const auto& subfolder :
                      fs::directory_iterator( folder, fs::directory_options::skip_permission_denied ) ) {
                    if ( !fs::is_directory( subfolder ) ) continue;

                    auto target2 = resolve_save_games( subfolder );
                    if ( !target2.has_value( ) ) {
                        for ( const auto& subsubfolder :
                              fs::directory_iterator( subfolder, fs::directory_options::skip_permission_denied ) ) {
                            if ( !fs::is_directory( subsubfolder ) ) continue;

                            auto target3 = resolve_save_games( subsubfolder );
                            if ( !target3.has_value( ) ) continue; // whatever
                            SPDLOG_INFO( "[Unreal] scanning target: {}", target3.value( ).string( ) );
                            auto found_games = scan_recursive( target3.value( ), translations );
                            std::ranges::move( found_games, std::back_inserter( games ) );
                        }
                    } else {
                        SPDLOG_INFO( "[Unreal] scanning target: {}", target2.value( ).string( ) );
                        auto found_games = scan_recursive( target2.value( ), translations );
                        std::ranges::move( found_games, std::back_inserter( games ) );
                    }
                }
            } else {
                SPDLOG_INFO( "[Unreal] scanning target: {}", target.value( ).string( ) );
                auto found_games = scan_recursive( target.value( ), translations );
                std::ranges::move( found_games, std::back_inserter( games ) );
            }
        }
    } catch ( const fs::filesystem_error& ex ) {
        SPDLOG_ERROR( "[Unreal] failed in {} because: {}", ex.path1( ).string( ), ex.code( ).message( ) );
    }

    return games;
}

std::vector<Game> CUnrealDetector::scan_recursive( const fs::path& path, const Translations& translations ) {
    static char header[4] = { 'G', 'V', 'A', 'S' };
    std::vector<Game> games;
    std::set<fs::path> directories;

    for ( const auto& entry :
          fs::recursive_directory_iterator( path, fs::directory_options::skip_permission_denied ) ) {
        if ( entry.path( ).extension( ) != ".sav" ) continue;

        std::ifstream save( entry.path( ), std::ifstream::binary );
        if ( !save.is_open( ) ) continue;

        char buffer[4];
        save.read( buffer, 4 );
        if ( save.gcount( ) != 4 ) continue;

        if ( !std::ranges::equal( buffer, header ) ) {
            SPDLOG_WARN( "[Unreal] {} does not contain a 'GVAS' header at the first 4 bytes, it might be custom. skipping..", entry.path( ).string( ) );
            continue;
        }

        // cursed skip method, could just remove them
        std::string path_str = entry.path( ).parent_path( ).string( );
        if ( path_str.find( "Application Data BACKUP" ) != std::string::npos ||
             path_str.find( "Settings" ) != std::string::npos ) {
            continue;
        }

        directories.insert( entry.path( ).parent_path( ) );
    }

    for ( const auto& entry : directories ) {
        auto it = entry.begin( );
        Game game;
        game.type = PlatformType::UNREAL;
        game.save_paths.push_back( entry );
        std::string found_name;

        std::vector<std::string> path_comps;
        std::ranges::transform(
            entry, std::back_inserter( path_comps ), []( const auto& part ) { return part.string( ); } );
        auto comp_it = std::ranges::find( path_comps, "SaveGames" );

        while ( comp_it != path_comps.begin( ) ) {
            --comp_it;
            bool is_numeric = std::ranges::all_of( *comp_it, ::isdigit );

            if ( *comp_it != "Saved" && *comp_it != "Steam" && *comp_it != "Epic" && !is_numeric ) {
                found_name = *comp_it;
                game.game_name = found_name;
                game.appid = "N/A";
                break;
            }
        }

        for ( ; it != entry.end( ); ++it ) {
            if ( *it == "compatdata" ) {
                ++it; // next component is the appid
                if ( it != entry.end( ) ) {
                    std::string appid = it->string( );

                    game.game_name = translations.get_steam_name( appid ).value_or( "N/A" );
                    game.appid = appid;
                    break;
                }
            } else if ( *it == "default" ) {
                ++it;
                if ( it != entry.end( ) ) {
                    std::string name = it->string( );

                    game.game_name = name;
                    game.appid = "N/A";
                    break;
                }
            }
        }

        if ( game.game_name.empty( ) && !found_name.empty( ) ) {
            game.game_name = found_name;
            game.appid = "N/A";
        }
        
        SPDLOG_INFO( "[Unreal] found: {}", game.game_name );
        games.push_back( game );
    }

    return games;
}

std::optional<fs::path> CUnrealDetector::resolve_save_games( const fs::path& folder ) {
    fs::path save_games = folder / "Saved" / "SaveGames";
    fs::path save_games_alt = folder / "SaveGames";
    std::optional<fs::path> target = std::nullopt;

    if ( fs::exists( save_games ) ) {
        target = save_games;
    } else if ( fs::exists( save_games_alt ) ) {
        target = save_games_alt;
    }

    return target;
}
