#pragma once
#include "detection/detection.hpp"
namespace fs = std::filesystem;

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

    std::unordered_map<std::string, std::string> load_labels( const std::string& game );
    void save_label( const std::string& game, const std::string& filename, const std::string& label );
    bool save_labels( const std::string& game, const std::unordered_map<std::string, std::string>& labels );

    void migrate_labels_to_metadata( );
}; // namespace Features
