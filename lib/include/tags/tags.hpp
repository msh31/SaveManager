#pragma once
#include <sm_error.hpp>

/*
    TODO LIST

    1. introduce big bad telemetry collection to phase out things like this in the future more easily, but would go
   against what i want for this program
*/

struct TagCache {
        std::vector<std::string> tags;
        std::string display;
};

namespace Tags {
    void migrate_labels_to_tags( ); // 1.

    std::unordered_map<std::string, std::vector<std::string>> load_tags( const std::string& game );

    std::expected<bool, SMError>
    save_tags( const std::string& game, const std::string& filename, const std::vector<std::string>& tags );

    bool delete_tags( const std::string& game, const std::string& filename );

    std::unordered_map<std::string, TagCache> load_tag_cache( const std::string& game_name );
}; // namespace Tags