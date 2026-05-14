#pragma once
#include <constants.hpp>

#include "config/config.hpp"
// #include "plugin/plugin.hpp"
#include "frontend/views/editor/editor_view.hpp"
#include "frontend/views/settings/settings.hpp"
#include "frontend/views/log/log.hpp"
#include "frontend/views/transfer/transfer.hpp"
#include "frontend/views/about/about.hpp"
#include "frontend/views/dashboard/dashboard.hpp"
// #include "frontend/views/backups/backup_view.hpp"
#include "frontend/views/about/about.hpp"

class App {
public:
    App(fs::path config_dir = paths::config_dir());
    ~App();

    void init();
    void render();

    GLFWwindow* window = nullptr;
    std::future<void> are_we_ready;

    TabState state;
private:
    void init_background();
    bool is_bg_initialized = false;

    GLuint compile_shader(const fs::path& path, GLenum type);
    GLuint compile_shader(const char* source, GLenum type);
    GLuint link_program(GLuint vert, GLuint frag);
    void init_quad();
    GLuint m_vao;
    GLuint m_vbo;
    GLuint m_shader_program;
    GLint m_u_resolution;
    GLint m_u_time;

    bool setup_opengl();
    bool setup_imgui();

    bool initialized = false;
    bool last_dark_mode = false;

    void render_ui();
    void render_loading_screen();

    Fonts fonts;
    // Plugin plugin;

    Config config;
    DashboardTab dahsboard_tab;
    // BackupTab backup_tab;
    EditorTab editor_tab;
    TransferTab transfer_tab;
    LogTab log_tab;
    AboutTab about_tab;
    SettingsTab settings_tab;

    Detection::DetectionResult d_result;
    std::future<void> detection_future;
};
