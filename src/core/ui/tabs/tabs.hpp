#pragma once
#include "../../globals.hpp"
#include "../../../detection/detection.hpp"
#include <GL/gl.h>

namespace Tabs {
    void render_general_tab(const Fonts& fonts, const Detection::DetectionResult& result, GLuint texture_id);
    void render_log_tab(const Fonts& fonts);
    void render_about_tab(const Fonts& fonts);
};
