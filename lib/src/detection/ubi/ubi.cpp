#include "detection/ubi/ubi.hpp"
#include "utils/translations/translations.hpp"

std::string_view CUbisoftDetector::name( ) const { return "Ubisoft"; }

std::expected<std::vector<Game>, SMError> CUbisoftDetector::find( ) {
    std::vector<fs::path> prefixes = { };
#ifdef _WIN32
    prefixes.emplace_back( "C:\\Program Files (x86)\\Ubisoft\\Ubisoft Game Launcher\\savegames" ); // TODO: improve this

    prefixes.emplace_back( paths::home_dir( ) / "AppData" / "Roaming" ); // anno
    prefixes.emplace_back( paths::documents_dir( ) );                    // anno
#endif
#ifdef __linux__
// todo
#endif

    std::vector<Game> games = { };

    for ( const auto& prefix : prefixes ) {
        if ( !fs::exists( prefix ) ) continue;

        for ( const auto& uuid : fs::directory_iterator( prefix, fs::directory_options::skip_permission_denied ) ) {
            fs::path    uuid_folder = uuid.path( );
            std::string folder_name = uuid_folder.filename( ).string( );

            if ( auto it = m_anno_paths.find( folder_name ); it != m_anno_paths.end( ) ) {
                auto& [key, anno_data] = *it;

                Game anno;
                anno.type             = PlatformType::UBISOFT;
                anno.game_name        = anno_data.game_name;
                anno.appid            = translations::get_steam_id( anno_data.game_name ).value_or( "N/A" );
                anno.show_parent_path = true;

                if ( fs::exists( uuid.path( ) / "accounts" ) ) {
                    for ( const auto& entry : fs::directory_iterator(
                              uuid.path( ) / "accounts", fs::directory_options::skip_permission_denied ) ) {
                        anno.save_paths.push_back( entry ); // a fallback
                        break;
                    }
                } else {
                    fs::path save = uuid.path( ) / anno_data.save_subpath;
                    anno.save_paths.push_back( fs::exists( save ) ? save : uuid.path( ) ); // a fallback
                }

                games.push_back( anno );
            }

            if ( !fs::is_directory( uuid_folder ) ) {
                continue;
            }

            for ( const auto& game_entry : fs::directory_iterator( uuid_folder ) ) {
                fs::path game_id_folder = game_entry.path( );
                if ( !fs::is_directory( game_id_folder ) ) {
                    continue;
                }

                auto name = translations::get_game_name_ubi( game_id_folder.filename( ).string( ) );
                if ( !name.has_value( ) ) {
                    continue;
                }

                Game game;
                game.type      = PlatformType::UBISOFT;
                game.game_id   = game_id_folder.filename( ).string( );
                game.game_name = name.value( );
                game.appid     = translations::get_steam_id( game.game_name ).value_or( "N/A" );
                game.save_paths.push_back( game_id_folder );

                games.push_back( game );
            }
        }
    }
    return games;
}
