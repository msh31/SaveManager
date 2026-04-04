#pragma once
#include "backend/detection/detection.hpp"

class Config;

namespace Features {
void backup_game(const Game& game, Config& config);
void restore_backup(const fs::path& name, const Game& selected_game);
std::vector<fs::path> get_backups(const Game& game, Config& config);

std::string construct_backup_name(const Game& game, const std::string& custom_name = "");

std::unordered_map<std::string, std::string> load_labels(const Game& game, Config& config);
void save_label(const Game& game, Config& config, const std::string& filename, const std::string& label);
void save_labels(const Game& game, Config& config, const std::unordered_map<std::string, std::string>& labels);
};
