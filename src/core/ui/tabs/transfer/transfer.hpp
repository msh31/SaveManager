#pragma once
#include "core/globals.hpp"
#include "core/detection/detection.hpp"

class Config;

namespace TransferTab {
void render(const Fonts& fonts, const Detection::DetectionResult& result, Config& config);
};
