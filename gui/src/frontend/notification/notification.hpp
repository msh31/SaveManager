#pragma once

/*
 * Code was copied from my other project here:
 * https://github.com/msh31/SaveManager/tree/main/src/frontend/ui/notifications
 *
 * It's GPLv3, but me being the author of both, i dont care nor for any projects derived from this by me.
 */

namespace Notify {
    struct Notification {
            std::string                           title;
            std::string                           text;
            float                                 duration;
            float                                 last_height = 0.f;
            std::chrono::steady_clock::time_point start_time;
    };

    void show_notification( const std::string& title, const std::string& text, int duration_ms );
    void render_notifications( );
}; // namespace Notify
