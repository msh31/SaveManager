#pragma once
#include "core/globals.hpp"
#include "core/detection/detection.hpp"
#include <GL/gl.h>
#include <unordered_map>

namespace Tabs {
    void render_general_tab(const Fonts& fonts, const Detection::DetectionResult& result, std::unordered_map<std::string, GLuint> texture_id);
    void render_log_tab(const Fonts& fonts);
    void render_about_tab(const Fonts& fonts);
};
