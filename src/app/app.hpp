#pragma once
#include "backend/config/config.hpp"
#include "frontend/views/editor/editor_view.hpp"
#include "frontend/views/settings/settings.hpp"
#include "frontend/views/log/log.hpp"
#include "frontend/views/transfer/transfer.hpp"
#include "frontend/views/about/about.hpp"
#include "frontend/views/dashboard/dashboard.hpp"
#include "frontend/views/about/about.hpp"

// #include "frontend/ui/textures/textures.hpp"

class App {
public:
    ~App();

    void init();
    void render();

    GLFWwindow* window = nullptr;
    std::future<void> are_we_ready;
    // bool refresh_requested;

    TabState state;
private:
    bool setup_opengl();
    bool setup_imgui();

    bool initialized = false;
    bool last_dark_mode = true;

    void render_ui();
    void render_loading_screen();

    Fonts fonts;

    Config config;
    DashboardTab dahsboard_tab;
    EditorTab editor_tab;
    TransferTab transfer_tab;
    LogTab log_tab;
    AboutTab about_tab;
    SettingsTab settings_tab;

    // std::unordered_map<std::string, GLuint> game_textures;
    // std::vector<std::future<Textures::ImageData>> texture_futures;

    Detection::DetectionResult d_result;
};
