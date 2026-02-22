#pragma once
#include "../../globals.hpp"
#include "../../../detection/detection.hpp"

namespace Tabs {
    void render_general_tab(const Fonts& fonts, const Detection::DetectionResult& result);
    void render_log_tab(const Fonts& fonts);
    void render_about_tab(const Fonts& fonts);
};
