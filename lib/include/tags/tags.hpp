#pragma once
#include <sm_error.hpp>

/*
    TODO LIST

    1. introduce big bad telemetry collection to phase out things like this in the future more easily, but would go
   against what i want for this program
*/

namespace Tags {
    void migrate_labels_to_tags( ); // 1.

    std::unordered_map<std::string, std::vector<std::string>> load_tags( const std::string& game );

    std::expected<bool, SMError>
    save_tags( const std::string& game, const std::string& filename, const std::vector<std::string>& tags );

    bool delete_tags( const std::string& game, const std::string& filename );
}; // namespace Tags