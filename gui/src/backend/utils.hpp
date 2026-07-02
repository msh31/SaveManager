#pragma once
#include <features/features.hpp>
#include <utils/utils.hpp>

inline std::unordered_map<std::string, TagCache> load_tag_cache( const std::string& game_name ) {
    std::unordered_map<std::string, TagCache> cache;

    auto loaded_tags = Features::load_tags( game_name );
    std::string file_name = ( paths::backup_dir( ) / sanitize_filename( game_name ) / "tags.json" ).string( );

    if ( loaded_tags.empty( ) ) {
        if ( fs::exists( file_name ) ) SPDLOG_WARN( "Failed to load tags for: {}", game_name );
        return { };
    }

    for ( const auto& [filename, tags] : loaded_tags ) {
        TagCache tcache;
        tcache.tags = tags;
        tcache.display =
            tags | std::ranges::views::join_with( std::string_view( ", " ) ) | std::ranges::to<std::string>( );
        cache.insert( { filename, tcache } );
    }

    return cache;
}
