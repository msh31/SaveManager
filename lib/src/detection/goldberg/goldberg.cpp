#include "goldberg.hpp"
#include "../detector_utils.hpp"
#include <logger.hpp>

std::string_view CGoldbergDetector::name( ) const { return PLATFORM_LABEL; }

std::expected<std::vector<Game>, SMError> CGoldbergDetector::find( ) {
    std::vector<fs::path> prefixes = { };
//#ifdef _WIN32
//    prefixes.emplace_back(
//        "C:\\Program Files (x86)\\Ubisoft\\Ubisoft Game Launcher\\savegames" ); // TODO: improve this (edit: how? wtf)
//
//    prefixes.emplace_back( paths::home_dir( ) / "AppData" / "Roaming" ); // anno - very broad search
//    prefixes.emplace_back( paths::documents_dir( ) );                    // anno - very broad search
//#endif
//
//    return scan_prefixes( PLATFORM_LABEL, prefixes, [this]( const fs::path& p ) { return scan( p, m_translations ); } );

    return { };
}

std::vector<Game> CGoldbergDetector::scan_wine_user( const fs::path& user_home, const DetectorContext& ctx ) {
    std::vector<Game> games;
    //auto documents = scan( user_home / "Documents", ctx.translations );
    //auto anno_alt = scan( user_home / "AppData" / "Roaming", ctx.translations );
    //std::ranges::move( documents, std::back_inserter( games ) );
    //std::ranges::move( anno_alt, std::back_inserter( games ) );
    return games;
}

std::vector<Game> CGoldbergDetector::scan_wine_prefix( const fs::path& drive_c, const DetectorContext& ctx ) {
    //fs::path launcher_path = drive_c / "Program Files (x86)" / "Ubisoft" / "Ubisoft Game Launcher" / "savegames";
    //return scan( launcher_path, ctx.translations );
    return { };
}

std::vector<Game> CGoldbergDetector::scan( fs::path path, const Translations& translations ) {
    if ( !fs::exists( path ) ) return { };
    std::vector<Game> games = { };


    return games;
}
