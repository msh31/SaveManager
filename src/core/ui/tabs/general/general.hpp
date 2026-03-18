#pragma once
#include "core/globals.hpp"
#include "core/detection/detection.hpp"
#include "core/features/features.hpp"

namespace GeneralTab {
void render(const Fonts& fonts, const Detection::DetectionResult& result, std::unordered_map<std::string, GLuint> texture_id, Config& config);

static bool open_restore_modal, open_delete_modal = false;
static std::vector<fs::path> backups;
static const Game* pending_restore_game = nullptr;
static const Game* pending_delete_game = nullptr;
static int selected_backup_idx = 0;
};
