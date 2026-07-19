#include "ubi.hpp"

std::string_view CUbisoftDetector::name( ) const { return PLATFORM_LABEL; }

std::expected<std::vector<Game>, SMError> CUbisoftDetector::find( ) {
    std::vector<fs::path> prefixes = { };
#ifdef _WIN32
    prefixes.emplace_back( "C:\\Program Files (x86)\\Ubisoft\\Ubisoft Game Launcher\\savegames" ); // TODO: improve this (edit: how? wtf)

    prefixes.emplace_back( paths::home_dir( ) / "AppData" / "Roaming" ); // anno - very broad search
    prefixes.emplace_back( paths::documents_dir( ) );                    // anno - very broad search
#endif

    std::vector<Game> games = { };

    for ( const auto& prefix : prefixes ) {
        if ( !fs::exists( prefix ) ) continue;
        SPDLOG_INFO( "[Ubisoft] searching prefix: {}", prefix.string() );

        auto found_games = scan( prefix, m_translations );
        std::ranges::move( found_games, std::back_inserter( games ) );
    }
    return games;
}

std::vector<Game> CUbisoftDetector::scan( fs::path path, const Translations& translations ) {
    if ( !fs::exists( path ) ) return { };
    std::vector<Game> games = { };

    static const std::unordered_map<std::string_view, AnnoEntry> anno_paths = {
        { "Anno 117 - Pax Romana", { "Anno 117: Pax Romana", "accounts" } },
        { "Anno 1800", { "Anno 1800", "accounts" } },
        { "Anno 1404", { "Anno 1404", "Savegames" } },
        { "Anno1404", { "Anno 1404", "Profiles" } },
        { "Anno 1404 Venice", { "Anno 1404 Venice", "Savegames" } },
        { "Anno 1503 History Edition", { "Anno 1503 History Edition", "SaveGame" } },
        { "Anno 1602 History Edition", { "Anno 1602 History Edition", "" } },
        { "Anno 1701 History Edition", { "Anno 1701 History Edition", "" } },
        { "Anno 2070", { "Anno 2070", "Accounts" } },
    };

    for ( const auto& uuid : fs::directory_iterator( path, fs::directory_options::skip_permission_denied ) ) {
        fs::path uuid_folder = uuid.path( );
        std::string folder_name = uuid_folder.string( );
        //SPDLOG_INFO( "[Ubisoft] looking for saves in {}", folder_name );

        if ( auto it = anno_paths.find( folder_name ); it != anno_paths.end( ) ) {
            auto& [key, anno_data] = *it;

            Game anno;
            anno.type = PlatformType::UBISOFT;
            anno.platform_label = std::string( PLATFORM_LABEL );
            anno.game_name = anno_data.game_name;
            anno.show_parent_path = true;

            fs::path expected_path = uuid.path( ) / "accounts";
            if ( fs::exists(expected_path ) ) {
                SPDLOG_INFO( "[Ubisoft] found account ID: {}", uuid.path( ).string() ); //??
                for ( const auto& entry : fs::directory_iterator(
                          uuid.path( ) / "accounts", fs::directory_options::skip_permission_denied ) ) {
                    
                    anno.save_paths.push_back( entry ); // a fallback
                    break;
                }
            } else {
                fs::path save = uuid.path( ) / anno_data.save_subpath;
                SPDLOG_INFO(
                    "[Ubisoft] failed to find {} falling back to: {}", expected_path.string( ), save.string( ) );
                anno.save_paths.push_back( fs::exists( save ) ? save : uuid.path( ) ); // a fallback
            }

            games.push_back( anno );
        }

        if ( !fs::is_directory( uuid_folder ) ) {
            continue;
        }

        for ( const auto& game_entry :
              fs::directory_iterator( uuid_folder, fs::directory_options::skip_permission_denied ) ) {
            fs::path game_id_folder = game_entry.path( );
            if ( !fs::is_directory( game_id_folder ) ) {
                continue;
            }
            //SPDLOG_INFO( "[Ubisoft] scanning: {}", game_id_folder.string() );

            auto gid = game_id_folder.filename( ).string( );
            auto name = translations.get_game_name_ubi( gid );
            if ( !name.has_value( ) ) {
                //SPDLOG_ERROR( "[Ubisoft] failed to find name for {}, skipping..", gid );
                continue;
            }

            Game game;
            game.type = PlatformType::UBISOFT;
            game.platform_label = std::string( PLATFORM_LABEL );
            game.game_id = game_id_folder.filename( ).string( );
            game.game_name = name.value( );
            game.save_paths.push_back( game_id_folder );

            SPDLOG_INFO( "[Ubisoft] found: {}", name.value( ) );
            games.push_back( game );
        }
    }

    return games;
}
