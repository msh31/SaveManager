#pragma once
#include "core/ui/notifications/notification.hpp"
#include "core/network/network.hpp"
#include "core/helpers/remote_transfer/remote_transfer.hpp"
#include "core/features/features.hpp"
#include "core/globals.hpp"

#include "imgui/misc/cpp/imgui_stdlib.h"

namespace TransferTab {
void render(const Fonts& fonts, const Detection::DetectionResult& result, Config& config);
};
