#include "wine.hpp"

std::string_view CWinePrefixDetector::name( ) const { return "Wine"; }

std::expected<std::vector<Game>, SMError> CWinePrefixDetector::find( ) {
    if ( !fs::exists( m_path ) ) return { };

    std::vector<Game> games = { };
    for ( const auto& entry : fs::directory_iterator( m_path ) ) {
        try {
            fs::path prefix = entry.path( );
            if ( !fs::exists( prefix ) ) continue;

            fs::path drive_c = fs::exists( prefix / "pfx" ) ? prefix / "pfx/drive_c" : prefix / "drive_c";
            fs::path users_dir = drive_c / "users";

            if ( !fs::exists( users_dir ) ) continue;

            for ( auto& hook : m_prefix_hooks ) {
                auto found = hook( drive_c, m_ctx );
                std::ranges::move( found, std::back_inserter( games ) );
            }

            for ( const auto& user : fs::directory_iterator( users_dir ) ) {
                if ( user.path( ).filename( ) == "Public" ) continue;

                for ( auto& hook : m_user_hooks ) {
                    auto found = hook( user.path( ), m_ctx );
                    std::ranges::move( found, std::back_inserter( games ) );
                }
            }
        } catch ( const fs::filesystem_error& fse ) {
            // TODO: do something with this, logging is spammy on linux
            //  SPDLOG_WARN( "filesystem error occured, skipping {}: {}", entry.path( ).string( ), fse.what( ) );
        }
    }

    return games;
}
