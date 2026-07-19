#include "ea.hpp"

namespace {
    constexpr std::array<char, 4> NFS_HEADER = { 'N', 'F', 'S', 'S' };
    constexpr std::array<char, 4> HPR_HEADER = { '\x00', '\x00', '\x04', '\x00' };

    bool folder_contains_header( const fs::path& path, const std::array<char, 4>& header ) {
        try {
            for ( const auto& entry :
                  fs::recursive_directory_iterator( path, fs::directory_options::skip_permission_denied ) ) {
                if ( !fs::is_regular_file( entry ) ) continue;

                std::ifstream file( entry.path( ), std::ifstream::binary );
                if ( !file.is_open( ) ) continue;

                std::array<char, 4> buffer{ };
                file.read( buffer.data( ), 4 );
                if ( file.gcount( ) == 4 && buffer == header ) return true;
            }
        } catch ( const fs::filesystem_error& ex ) {
            SPDLOG_WARN( "[EA] failed to scan {}: {}", path.string( ), ex.what( ) );
        }
        return false;
    }

    Game make_game( std::string name, fs::path save_path ) {
        Game game;
        game.type = PlatformType::EA;
        game.platform_label = std::string( CElectronicArtsDetector::PLATFORM_LABEL );
        game.game_name = std::move( name );
        game.appid = "N/A";
        game.save_paths.push_back( std::move( save_path ) );
        game.show_parent_path = true; // save folders hold generically-named files (e.g. "savegame/1", "wraps/...")
        return game;
    }
} // namespace

std::string_view CElectronicArtsDetector::name( ) const { return PLATFORM_LABEL; }

std::expected<std::vector<Game>, SMError> CElectronicArtsDetector::find( ) {
    std::unordered_map<SaveRoot, fs::path> roots;
#ifdef _WIN32
    roots[SaveRoot::DOCUMENTS] = save::resolve_root( SaveRoot::DOCUMENTS );
    roots[SaveRoot::LOCAL_APPDATA] = save::resolve_root( SaveRoot::LOCAL_APPDATA );
    roots[SaveRoot::PROGRAM_DATA] = save::resolve_root( SaveRoot::PROGRAM_DATA );
#endif

    auto games = scan( roots );
    auto legacy = scan_legacy( roots );
    std::ranges::move( legacy, std::back_inserter( games ) );
    return games;
}

std::vector<Game> CElectronicArtsDetector::scan_wine_user( const fs::path& user_home, const DetectorContext& ctx ) {
    std::unordered_map<SaveRoot, fs::path> roots = {
        { SaveRoot::DOCUMENTS, user_home / "Documents" },
        { SaveRoot::LOCAL_APPDATA, user_home / "AppData" / "Local" },
    };

    auto games = scan( roots );
    auto legacy = scan_legacy( roots );
    std::ranges::move( legacy, std::back_inserter( games ) );
    return games;
}

std::vector<Game> CElectronicArtsDetector::scan_wine_prefix( const fs::path& drive_c, const DetectorContext& ctx ) {
    std::unordered_map<SaveRoot, fs::path> roots = { { SaveRoot::PROGRAM_DATA, drive_c / "ProgramData" } };

    auto games = scan( roots );
    auto legacy = scan_legacy( roots );
    std::ranges::move( legacy, std::back_inserter( games ) );
    return games;
}

std::vector<Game> CElectronicArtsDetector::scan( const std::unordered_map<SaveRoot, fs::path>& roots ) {
    static const std::vector<SaveLocation> table = {
        { "Need for Speed Payback", SaveRoot::DOCUMENTS, "Need for Speed(TM) Payback/SaveGame", NFS_HEADER },
        { "Need for Speed Heat", SaveRoot::DOCUMENTS, "Need for Speed Heat/SaveGame", NFS_HEADER },
        { "Need for Speed Unbound", SaveRoot::DOCUMENTS, "Need for Speed(TM) Unbound/SaveGame", NFS_HEADER },
        { "Need for Speed Hot Pursuit Remastered", SaveRoot::DOCUMENTS,
          "Criterion Games/Need for Speed(TM) Hot Pursuit Remastered/Save", HPR_HEADER },
        { "Need for Speed Rivals", SaveRoot::DOCUMENTS,
          "Ghost Games/Need for Speed(TM) Rivals/settings", // no readable header, so existence +
                                                            // non-empty is the best we can do

          std::nullopt },
    };

    std::vector<Game> games;
    for ( const auto& loc : table ) {
        auto root_it = roots.find( loc.root_path );
        if ( root_it == roots.end( ) ) continue;

        fs::path path = root_it->second / loc.relative_path;
        if ( !fs::exists( path ) ) continue;

        if ( loc.header_bytes ) {
            if ( !folder_contains_header( path, *loc.header_bytes ) ) continue;
        } else if ( fs::is_empty( path ) ) {
            continue;
        }

        SPDLOG_INFO( "[EA] found: {}", loc.game_name );
        games.push_back( make_game( loc.game_name, path ) );
    }

    return games;
}

std::vector<Game> CElectronicArtsDetector::scan_legacy( const std::unordered_map<SaveRoot, fs::path>& roots ) {
    std::vector<Game> games;

    // Underground 2: <root>/NFS Underground 2/<username>/<username> - save file shares its parent folder's name
    if ( auto it = roots.find( SaveRoot::LOCAL_APPDATA ); it != roots.end( ) ) {
        fs::path root = it->second / "NFS Underground 2";
        if ( fs::exists( root ) ) {
            try {
                for ( const auto& user_dir :
                      fs::directory_iterator( root, fs::directory_options::skip_permission_denied ) ) {
                    if ( !fs::is_directory( user_dir ) ) continue;
                    if ( !fs::exists( user_dir.path( ) / user_dir.path( ).filename( ) ) ) continue;

                    SPDLOG_INFO( "[EA] found: Need for Speed Underground 2" );
                    games.push_back( make_game( "Need for Speed Underground 2", user_dir.path( ) ) );
                }
            } catch ( const fs::filesystem_error& ex ) {
                SPDLOG_WARN( "[EA] failed to scan {}: {}", root.string( ), ex.what( ) );
            }
        }
    }

    // Underground: <root>/NFS Underground/*.ugd - flat save files, no dedicated save subfolder
    if ( auto it = roots.find( SaveRoot::PROGRAM_DATA ); it != roots.end( ) ) {
        fs::path root = it->second / "NFS Underground";
        if ( fs::exists( root ) ) {
            bool has_saves = false;
            try {
                for ( const auto& entry :
                      fs::directory_iterator( root, fs::directory_options::skip_permission_denied ) ) {
                    if ( entry.path( ).extension( ) == ".ugd" ) {
                        has_saves = true;
                        break;
                    }
                }
            } catch ( const fs::filesystem_error& ex ) {
                SPDLOG_WARN( "[EA] failed to scan {}: {}", root.string( ), ex.what( ) );
            }

            if ( has_saves ) {
                SPDLOG_INFO( "[EA] found: Need for Speed Underground" );
                games.push_back( make_game( "Need for Speed Underground", root ) );
            }
        }
    }

    return games;
}
