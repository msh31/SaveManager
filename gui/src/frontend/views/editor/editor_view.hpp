#pragma once
#include "features/save_editor/save_editor.hpp"
#include "utils/utils.hpp"

struct EditorTab {
    void render( const Fonts &fonts );
    std::string file_path;

    SanAndreas san_andreas;
};
