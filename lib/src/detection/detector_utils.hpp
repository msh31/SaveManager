#pragma once
#include <detection/game.hpp>

template <typename ScanFn>
std::vector<Game> scan_prefixes( std::string_view label, const std::vector<fs::path>& prefixes, ScanFn&& scan ) {
    std::vector<Game> games;
    for ( const auto& prefix : prefixes ) {
        if ( !fs::exists( prefix ) ) continue;
        SPDLOG_INFO( "[{}] searching prefix: {}", label, prefix.string( ) );

        auto found = scan( prefix );
        std::ranges::move( found, std::back_inserter( games ) );
    }
    return games;
}

// Steam names each compatdata subfolder after the appid it belongs to, e.g.
// steamapps/compatdata/<appid>/pfx/drive_c/users/<user>. Recover that appid from a
// user_home passed to a wine-user hook, so scan_wine_user only resolves the one game
// this prefix actually is, instead of every manifest entry on every prefix.
static std::optional<uint32_t> resolve_prefix_appid( const fs::path& user_home ) {
    fs::path drive_c = user_home.parent_path( ).parent_path( );
    fs::path prefix = drive_c.parent_path( );
    if ( prefix.filename( ) == "pfx" ) prefix = prefix.parent_path( );

    try {
        return static_cast<uint32_t>( std::stoul( prefix.filename( ).string( ) ) );
    } catch ( const std::exception& ) {
        return std::nullopt;
    }
}
