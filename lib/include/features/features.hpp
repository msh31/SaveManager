#pragma once
#include "detection/detection.hpp"

class CConfig;

namespace Features {
    bool backup_game( const Game& game, const fs::path& file, CConfig& config );
    std::vector<std::string> backup_all_games( const std::vector<Game>& snapshot, CConfig& config );
    bool backup_to_path( fs::path source, fs::path dest );
    bool backup_game_files( const Game& game, std::vector<std::pair<fs::path, const Game*>> files );

    bool restore_backup(
        const fs::path& name, const fs::path& save_path, std::vector<std::pair<fs::path, fs::path>>& conflicts );
    std::vector<fs::path> get_backups( const std::string& game );

    std::string construct_backup_name( const std::string& game, const std::string& custom_name = "" );

    // TODO: introduce big bad telemetry collection to phase out shit like this in the future more easily
    // COMMENT: this needs to be thought about more
    void migrate_labels_to_tags( ); // not a feature
    std::unordered_map<std::string, std::vector<std::string>> load_tags( const std::string& game );
    std::expected<bool, SMError>
    save_tags( const std::string& game, const std::string& filename, const std::vector<std::string>& tags );
    bool delete_tags( const std::string& game, const std::string& filename );
} // namespace Features
