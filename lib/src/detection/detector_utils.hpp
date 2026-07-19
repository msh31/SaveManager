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
