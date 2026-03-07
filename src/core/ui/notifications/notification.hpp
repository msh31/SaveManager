#pragma once
#include <chrono>
#include <string>
#include <vector>

namespace Notify {
struct Notification {
    std::string title;
    std::string text;
    float duration;
    float last_height = 0.f;
    std::chrono::steady_clock::time_point start_time;
};

void show_notification(const std::string& title, const std::string& text, int duration_ms);
void render_notifications();
};

