#pragma once
#include <zip.h>

#include "../../detection/detection.hpp"
#include "../../helpers/utils.hpp"

class Features {
public:
    static void backup_game(const Game& game);
    static void restore_game_backup(const Game& game);
};
