#pragma once
#include <filesystem>
#include <string>

#include "../../helpers/utils.hpp"

#include <zip.h>

namespace Backup {
    void create_backup(const fs::path& name, const Game& selected_game);
    void restore_backup(const fs::path& name, const Game& selected_game);
}
