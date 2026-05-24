#pragma once
#include "detection/detection.hpp"
namespace fs = std::filesystem;

class CConfig;

namespace Features {
void backup_game( const Game &game, const fs::path &file, CConfig &config );
void backup_all_games( std::vector<Game> snapshot, CConfig &config );
void backup_to_path( fs::path source, fs::path dest );
bool backup_game_files( const Game &game, std::vector<std::pair<fs::path, const Game *>> files );

void restore_backup( const fs::path &name, const fs::path &save_path,
                     std::vector<std::pair<fs::path, fs::path>> &conflicts );
std::vector<fs::path> get_backups( const std::string &game, CConfig &config );

std::string construct_backup_name( const std::string &game, const std::string &custom_name = "" );

std::unordered_map<std::string, std::string> load_labels( const std::string &game, CConfig &config );
void save_label( const std::string &game, CConfig &config, const std::string &filename, const std::string &label );
void save_labels( const std::string &game, CConfig &config, const std::unordered_map<std::string, std::string> &labels );
}; // namespace Features
