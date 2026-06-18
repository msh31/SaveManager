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

        auto found_games = scan( prefix );
        std::ranges::move( found_games, std::back_inserter( games ) );
    }

    return games;
}

// private
std::vector<Game> CUnrealDetector::scan( fs::path path ) {
    if ( !fs::exists( path ) ) return { };
    std::vector<Game> games;

    for ( const auto& folder : fs::directory_iterator( path, fs::directory_options::skip_permission_denied ) ) {
        if ( !fs::is_directory( folder ) ) continue;

        fs::path save_games = folder.path( ) / "Saved" / "SaveGames";
        fs::path save_games_alt = folder.path( ) / "SaveGames";
        fs::path target;

        if ( fs::exists( save_games ) ) {
            target = save_games;
        } else if ( fs::exists( save_games_alt ) ) {
            target = save_games_alt;
        } else {
            try {
                for ( const auto& subfolder :
                      fs::directory_iterator( folder, fs::directory_options::skip_permission_denied ) ) {
                    if ( !fs::is_directory( subfolder ) ) {
                        continue;
                    }
                    fs::path sub_save_games = subfolder.path( ) / "Saved" / "SaveGames";
                    fs::path sub_save_games_alt = subfolder.path( ) / "SaveGames";
                    fs::path target_two;

                    if ( fs::exists( sub_save_games ) ) {
                        target_two = sub_save_games;
                    } else if ( fs::exists( sub_save_games_alt ) ) {
                        target_two = sub_save_games_alt;
                    } else {
                        continue;
                    }

                    auto found_games = scan_recursive( target_two );
                    std::ranges::move( found_games, std::back_inserter( games ) );
                }
            } catch ( const fs::filesystem_error& ) {
                continue;
            }
            continue;
        }

        try {
            auto found_games = scan_recursive( target );
            std::ranges::move( found_games, std::back_inserter( games ) );
        } catch ( const fs::filesystem_error& ) {
            continue;
        }
    }

    return games;
}

std::vector<Game> CUnrealDetector::scan_recursive( const fs::path& path ) {
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

        if ( !std::ranges::equal( buffer, header ) ) continue;

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

                    game.game_name = translations::get_steam_name( appid ).value_or( "N/A" );
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
        games.push_back( game );
    }

    return games;
}
