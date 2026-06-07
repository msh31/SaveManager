#include "detection/ubi/ubi.hpp"
#include "utils/translations/translations.hpp"
// #include "logger/logger.hpp"

std::expected<std::vector<Game>, SMError> CUbisoftDetector::find_saves( const fs::path& prefix ) const {
    if ( !fs::exists( prefix ) ) {
        return std::unexpected{ SMError::PATH_NOT_FOUND };
    }
    std::vector<Game> games;
    // get_logger().debug("scanning path: {}", prefix.string());

    for ( const auto& uuid_entry :
          fs::directory_iterator( prefix, std::filesystem::directory_options::skip_permission_denied ) ) {
        fs::path uuid_folder = uuid_entry.path( );

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
            // game.save_path = game_id_folder;
            game.save_paths.push_back( game_id_folder );

            games.push_back( game );
        }
    }
    return games;
}

std::expected<std::vector<Game>, SMError> CUbisoftDetector::find_anno_saves( const fs::path& prefix ) const {
    if ( !fs::exists( prefix ) ) {
        return std::unexpected{ SMError::PATH_NOT_FOUND };
    }
    std::vector<Game> games;
    // get_logger().debug("scanning path: {}", prefix.string());

    for ( const auto& game :
          fs::directory_iterator( prefix, std::filesystem::directory_options::skip_permission_denied ) ) {
        std::string folder_name = game.path( ).filename( ).string( );

        if ( auto it = m_anno_paths.find( folder_name ); it != m_anno_paths.end( ) ) {
            // get_logger().info("Anno match: " + folder_name + " at " + game.path().string());
            auto& [key, anno_data] = *it;

            Game anno;
            anno.type             = PlatformType::UBISOFT;
            anno.game_name        = anno_data.game_name;
            anno.appid            = translations::get_steam_id( anno_data.game_name ).value_or( "N/A" );
            anno.show_parent_path = true;

            if ( fs::exists( game.path( ) / "accounts" ) ) {
                for ( const auto& entry : fs::directory_iterator(
                          game.path( ) / "accounts", std::filesystem::directory_options::skip_permission_denied ) ) {
                    // anno.save_path = entry;
                    anno.save_paths.push_back( entry ); // a fallback
                    break;
                }
            } else {
                fs::path save = game.path( ) / anno_data.save_subpath;
                // anno.save_path = fs::exists( save ) ? save : game.path( );             // a fallback
                anno.save_paths.push_back( fs::exists( save ) ? save : game.path( ) ); // a fallback
            }

            games.push_back( anno );
        }
    }
    return games;
}
