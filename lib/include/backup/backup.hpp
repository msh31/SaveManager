#pragma once
#include <detection/game.hpp>

#define MODE_CREATE_ARCHIVE ( ZIP_CREATE | ZIP_TRUNCATE )
#define MODE_EXTRACT_ARCHIVE 0

static const std::unordered_set<std::string_view> extension_blocklist{
    ".dat", ".bin",  ".upload", ".bak",  ".cfg",  ".log", ".tmp", ".ini",    ".set",
    ".txt", ".lock", ".lck",    ".part", ".temp", ".swp", ".swo", ".journal" };

static const std::unordered_set<std::string_view> g_extension_blocklist{ ".png", ".jpg", ".jpeg", ".webp", ".bmp" };

class CConfig;

namespace Backup {
    struct BackupEntry {
            fs::path name;
            fs::path save_path;
            std::vector<fs::path> entries;
            size_t size = 0;
    };

    bool backup_game( const Game& game, const fs::path& file, CConfig& config );
    std::vector<std::string> backup_all_games( const std::vector<Game>& snapshot, CConfig& config );
    bool backup_to_path( fs::path source, fs::path dest );
    bool backup_game_files( const Game& game, std::vector<std::pair<fs::path, const Game*>> files );

    bool restore_backup(
        const fs::path& name, const std::vector<fs::path>& save_paths,
        std::vector<std::pair<fs::path, fs::path>>& conflicts, std::unordered_set<std::string> exclusions = { } );
    std::vector<fs::path> get_backups( const std::string& game );

    std::string construct_backup_name( const std::string& game, const std::string& custom_name = "" );

    std::vector<std::string> get_backup_entries( const fs::path& backup );
}; // namespace Backup