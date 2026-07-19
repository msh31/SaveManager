#include "psstudios.hpp"
#include "../../utils/save_helper/save_helper.hpp"

namespace {
    const std::vector<SaveLocation>& table( ) {
        static const std::vector<SaveLocation> table = {
            { "Days Gone", SaveRoot::LOCAL_APPDATA, "BendGame/Saved", std::nullopt, true },
            { "Sackboy: A Big Adventure", SaveRoot::SAVED_GAMES, "Sackboy/Steam/SaveGames", std::nullopt,
              true }, // unsure if EGS needs EGS
            { "Death Stranding", SaveRoot::LOCAL_APPDATA, "KojimaProductions/DeathStranding", std::nullopt,
              true }, // user-id, Windows / Linux
            { "Death Stranding", SaveRoot::LOCAL_APPDATA,
              "Packages/505GAMESS.P.A.DeathStranding_tefn33qh9azfc/SystemAppData/xgs", std::nullopt,
              true }, // user-id, MS Store
            { "Death Stranding", SaveRoot::LOCAL_APPDATA_LOW, "KojimaProductions/DeathStranding", std::nullopt,
              true }, // user-id, EGS
            { "Death Stranding 2: On The Beach", SaveRoot::LOCAL_APPDATA, "DEATH STRANDING 2 - ON THE BEACH",
              std::nullopt, true }, // To be verified
            { "Horizon Zero Dawn", SaveRoot::DOCUMENTS, "Horizon Zero Dawn/Saved Game", std::nullopt, true },
            { "Horizon Zero Dawn Remastered", SaveRoot::DOCUMENTS, "Horizon Zero Dawn Remastered/", std::nullopt,
              true }, // user-id/*.dat
            { "Horizon Forbidden West", SaveRoot::DOCUMENTS, "Horizon Zero Dawn/Saved Game", std::nullopt, true },
            { "God of War", SaveRoot::SAVED_GAMES, "God of War", std::nullopt, true },
            { "God of War Ragnarök", SaveRoot::DOCUMENTS, "God of War Ragnarök", std::nullopt,
              true }, // has user-id subfodler
            { "Marvel's Spider-Man 2", SaveRoot::DOCUMENTS, "Marvel's Spider-Man 2", std::nullopt, true }, // user-id
            { "Marvel's Spider-Man Remastered", SaveRoot::DOCUMENTS, "Marvel's Spider-Man Remastered", std::nullopt,
              true }, // user-id
            { "The Last of Us Part II Remastered", SaveRoot::SAVED_GAMES, "The Last of Us Part II", std::nullopt,
              true }, // user-id/savedata..
            { "The Last of Us Part I", SaveRoot::SAVED_GAMES, "The Last of Us Part I/users", std::nullopt,
              true }, // user-id/savedata
            { "Until Dawn", SaveRoot::DOCUMENTS, "My Games/Bates/Saved/SaveGames", std::nullopt, true }, // user-id
            { "Kena: Bridge of Spirits", SaveRoot::LOCAL_APPDATA, "Kena/Saved/SaveGames/", std::nullopt, true },
            { "Stellar Blade", SaveRoot::LOCAL_APPDATA, "SB/Saved/SaveGames", std::nullopt, true },
        };
        return table;
    }

    Game make_game( std::string name, fs::path save_path ) {
        Game game;
        game.type = PlatformType::PLAYSTATION;
        game.platform_label = std::string( CPlaystationStudiosDetector::PLATFORM_LABEL );
        game.game_name = std::move( name );
        game.appid = "N/A";
        game.save_paths.push_back( std::move( save_path ) );
        game.show_parent_path = true;
        return game;
    }
} // namespace

std::string_view CPlaystationStudiosDetector::name( ) const { return PLATFORM_LABEL; }

std::expected<std::vector<Game>, SMError> CPlaystationStudiosDetector::find( ) {
    std::unordered_map<SaveRoot, fs::path> roots;
#ifdef _WIN32
    roots[SaveRoot::DOCUMENTS] = save::resolve_root( SaveRoot::DOCUMENTS );
    roots[SaveRoot::LOCAL_APPDATA] = save::resolve_root( SaveRoot::LOCAL_APPDATA );
    roots[SaveRoot::LOCAL_APPDATA_LOW] = save::resolve_root( SaveRoot::LOCAL_APPDATA_LOW );
    roots[SaveRoot::SAVED_GAMES] = save::resolve_root( SaveRoot::SAVED_GAMES );
#endif

    auto games = save::scan_locations( roots, table( ), PlatformType::PLAYSTATION, PLATFORM_LABEL );
    return games;
}

std::vector<Game> CPlaystationStudiosDetector::scan_wine_user( const fs::path& user_home, const DetectorContext& ctx ) {
    std::unordered_map<SaveRoot, fs::path> roots = {
        { SaveRoot::DOCUMENTS, user_home / "Documents" },
        { SaveRoot::LOCAL_APPDATA, user_home / "AppData" / "Local" },
        { SaveRoot::LOCAL_APPDATA_LOW, user_home / "AppData" / "LocalLow" },
        { SaveRoot::SAVED_GAMES, user_home / "Saved Games" },
    };

    auto games = save::scan_locations( roots, table( ), PlatformType::PLAYSTATION, PLATFORM_LABEL );
    return games;
}
