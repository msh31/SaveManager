#pragma once
#include "core/config/config.hpp"
#include "core/logger/logger.hpp"

#include "imgui.h"

struct Fonts {
    ImFont* regular;
    ImFont* medium;
    ImFont* bold;

    ImFont* title;
    ImFont* header;
};
