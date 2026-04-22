#pragma once
#include "backend/features/save_editor/save_editor.hpp"
#include "backend/utils/utils.hpp"

struct EditorTab {
    void render(const Fonts& fonts);
    std::string file_path; 

    SanAndreas san_andreas;
};
