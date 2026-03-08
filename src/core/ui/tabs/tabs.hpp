#pragma once
#include "core/config/config.hpp"
#include "core/globals.hpp"
#include "core/detection/detection.hpp"
#include <glad/glad.h>
#include <unordered_map>

namespace Tabs {
    void render_general_tab(const Fonts& fonts, const Detection::DetectionResult& result, std::unordered_map<std::string, GLuint> texture_id, Config& config);
    void render_log_tab(const Fonts& fonts);
    void render_about_tab(const Fonts& fonts);
    void render_settings_tab(const Fonts& fonts, Config& config);
    void render_debug_tab(const Fonts& fonts);
};
