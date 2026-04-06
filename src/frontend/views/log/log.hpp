#pragma once

struct LogTab {
    void render(const Fonts& fonts);

    double last_read_time = 0.0;
    std::string log_buffer;
};
