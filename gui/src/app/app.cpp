#ifdef _WIN32
#include <windows.h>
#pragma comment(linker, "/subsystem:windows /entry:mainCRTStartup")
#endif

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "app.hpp"

#include "frontend/ui/themes/themes.hpp"
#include "frontend/ui/notifications/notification.hpp"

#include "frontend/ui/fonts/jbm_reg.h"
#include "frontend/ui/fonts/jbm_med.h"
#include "frontend/ui/fonts/jbm_bold.h"
#include "frontend/ui/fonts/font_awesome.hpp"
#include <frontend/shaders/shader.hpp>

#include "detection/detection.hpp"
#include "utils/translations/translations.hpp"
#include "utils/blacklist/blacklist.hpp"

#include <curl/curl.h>

App::App(fs::path config_dir) : config(config_dir) {}

void App::init_background() {
    GLuint vert = compile_shader(default_vert, GL_VERTEX_SHADER);
    GLuint frag = compile_shader(default_frag, GL_FRAGMENT_SHADER);
    m_shader_program = link_program(vert, frag);
    m_u_resolution = glGetUniformLocation(m_shader_program, "iResolution");
    m_u_time = glGetUniformLocation(m_shader_program, "iTime");
    init_quad();
}

void App::init() {
    if(!setup_opengl()) {
        return;
    }
    if(!setup_imgui()) {
        return;
    }

    are_we_ready = std::async(std::launch::async, [this]() {
        if(!config.init()) {
            SPDLOG_ERROR("Config is missing and could not be generated!");
            Notify::show_notification("Config error!", "Config is missing and could not be generated!", 5000);
        }

        translations::init();
        Blacklist::init();
        config.save();
    });
    curl_global_init(CURL_GLOBAL_DEFAULT);

    glClearColor(0.145f, 0.145f, 0.141f, 1.0f);
}

void App::render_ui() {
    ImGui::AlignTextToFramePadding();

    if (ImGui::BeginTabBar("MyTabBar", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_DrawSelectedOverline)) {
        if (ImGui::BeginTabItem("Dashboard"))  {
            dahsboard_tab.render(fonts, d_result, config);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Save Editor"))  {
            editor_tab.render(fonts);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Transfer"))  {
            transfer_tab.render(fonts, d_result, config, state);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Log"))  {
            log_tab.render(fonts);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("About"))  {
            about_tab.render(fonts);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Settings"))  {
            settings_tab.render(fonts, config);
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}

void App::render_loading_screen() {
    ImVec2 center(
        (ImGui::GetWindowSize().x - 24) * 0.5f,
        (ImGui::GetWindowSize().y - 40) * 0.5f
    );
    ImGui::SetCursorPos(center);
    
    static float angle = 0.0f;
    angle += ImGui::GetIO().DeltaTime * 5.0f;
    
    ImVec2 spinner_center = ImGui::GetCursorScreenPos() + ImVec2(12, 12);
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    const int segments = 8;
    const float radius = 10.0f;
    const ImU32 base_color = ImGui::GetColorU32(ImGuiCol_Text);
    
    for (int i = 0; i < segments; i++) {
        float a = angle + (i * 2.0f * 3.14159f / segments);
        ImVec2 point(
            spinner_center.x + cosf(a) * radius,
            spinner_center.y + sinf(a) * radius
        );
        float alpha = (i + 1.0f) / segments;
        ImU32 segment_color = IM_COL32(
            200,
            200,
            200,
            (int)(255 * alpha)
        );
        draw_list->AddCircleFilled(point, 3, segment_color);
    }
    
    ImGui::SetCursorPos(ImVec2(
        (ImGui::GetWindowSize().x - ImGui::CalcTextSize("Loading").x) * 0.5f,
        center.y + 35
    ));
    ImGui::Text("Loading");
}

GLuint App::compile_shader(const fs::path& path, GLenum type) {
    GLint is_compiled = 0;

    std::ifstream shader(path);
    if(!shader.is_open()) return GL_FALSE;

    std::ostringstream shader_content; 
    shader_content << shader.rdbuf();
    std::string shader_source = shader_content.str(); 

    GLuint shader_id = glCreateShader(type);
    if(shader_id == 0) {
        SPDLOG_WARN("Failed to load shader: {}", path.string());
    }

    const char* src = shader_source.c_str();
    glShaderSource(shader_id, 1, &src, NULL);
    glCompileShader(shader_id);
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &is_compiled);

    if(is_compiled == GL_FALSE) {
        GLint log_length = 0;
        glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);
        std::string logs(log_length, '\0');
        glGetShaderInfoLog(shader_id, log_length, NULL, logs.data());
        SPDLOG_WARN("Shader compilation error: {} in: {}", logs, path.string());
        glDeleteShader(shader_id);
        return GL_FALSE;
    }
    return shader_id;
}

GLuint App::compile_shader(const char* source, GLenum type) {
    GLuint shader_id = glCreateShader(type);
    glShaderSource(shader_id, 1, &source, NULL);
    glCompileShader(shader_id);

    GLint is_compiled = 0;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &is_compiled);
    if(is_compiled == GL_FALSE) {
        GLint log_length = 0;
        glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);
        std::string logs(log_length, '\0');
        glGetShaderInfoLog(shader_id, log_length, NULL, logs.data());
        SPDLOG_WARN("Shader compilation error: {}", logs);
        glDeleteShader(shader_id);
        return GL_FALSE;
    }
    return shader_id;
}

GLuint App::link_program(GLuint vert, GLuint frag) {
    GLuint pid = glCreateProgram();
    GLint is_linked = 0;

    glAttachShader(pid, vert);
    glAttachShader(pid, frag);

    glLinkProgram(pid);

    glGetProgramiv(pid, GL_LINK_STATUS, &is_linked);
    if(is_linked == GL_FALSE) {
        GLint log_length = 0;
        glGetProgramiv(pid, GL_INFO_LOG_LENGTH, &log_length);
        std::string logs(log_length, '\0');
        glGetProgramInfoLog(pid, log_length, NULL, logs.data());
        SPDLOG_WARN("Shader linker error: {}", logs);

        glDeleteProgram(pid);

        glDeleteShader(vert);
        glDeleteShader(frag);
        return GL_FALSE;
    }

    glDetachShader(pid, vert);
    glDetachShader(pid, frag);

    glDeleteShader(vert);
    glDeleteShader(frag);
    return pid;
}

void App::init_quad() {
    float vertices[12] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f,  1.0f,
    };

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

bool App::setup_opengl() {
    if(!glfwInit()) {
        SPDLOG_ERROR("Failed to initialize GLFW.");
        return false;
    }

    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing (MSAA)
                   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // no old OpenGL

    window = glfwCreateWindow(DEF_RES_W, DEF_RES_H, APP_NAME, nullptr, nullptr);
    if(window == nullptr) {
        SPDLOG_ERROR("Failed to create GLFW window. OpenGL 3.3 support is required!");
        glfwTerminate();
        return false;
    }

    if(config.win_props.x != -1) {
        glfwSetWindowPos(window, config.win_props.x, config.win_props.y);
        glfwSetWindowSize(window, config.win_props.width, config.win_props.height);
    }

    glfwSetWindowSizeLimits(window, MIN_RES_W, MIN_RES_H, MAX_RES_W, MAX_RES_H); 
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    if (!gladLoadGL(glfwGetProcAddress)) {
        SPDLOG_ERROR("Failed to initialize GLAD");
        return false;
    }

    return true;
}

bool App::setup_imgui() {
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    ImFontConfig cfg_reg, cfg_med, cfg_small, cfg_bold, cfg_head, cfg_title, cfg_icons;
    cfg_reg.FontDataOwnedByAtlas = false;
    cfg_med.FontDataOwnedByAtlas = false;
    cfg_small.FontDataOwnedByAtlas = false;
    cfg_bold.FontDataOwnedByAtlas = false;
    cfg_head.FontDataOwnedByAtlas = false;
    cfg_title.FontDataOwnedByAtlas = false;

    cfg_icons.FontDataOwnedByAtlas = false;
    cfg_icons.MergeMode = true;
    cfg_icons.PixelSnapH = true;
    static const ImWchar icon_ranges[] = { 0xe000, 0xf8ff, 0 }; 
    cfg_icons.GlyphRanges = icon_ranges;

    fonts.regular = io.Fonts->AddFontFromMemoryTTF((void*)jbm_reg, jbm_reg_len, 20.0f, &cfg_reg);
    io.Fonts->AddFontFromMemoryTTF((void*)font_awesome, font_awesome_len, 18.0f, &cfg_icons);
    fonts.title = io.Fonts->AddFontFromMemoryTTF((void*)jbm_reg, jbm_reg_len, 34.0f, &cfg_title);
    fonts.small_font = io.Fonts->AddFontFromMemoryTTF((void*)jbm_reg, jbm_reg_len, 18.0f, &cfg_small);
    fonts.medium = io.Fonts->AddFontFromMemoryTTF((void*)jbm_med, jbm_med_len, 20.0f, &cfg_med);
    fonts.bold = io.Fonts->AddFontFromMemoryTTF((void*)jbm_bold, jbm_bold_len, 20.0f, &cfg_bold);
    fonts.header = io.Fonts->AddFontFromMemoryTTF((void*)jbm_reg, jbm_reg_len, 28.0f, &cfg_head);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    return true;
}

void App::render() {
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(window, &width, &height);

    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, width, height);

    if(config.settings.animated_background && !is_bg_initialized) {
        init_background();
        is_bg_initialized = true;
    }

    if(config.settings.animated_background) {
        glUseProgram(m_shader_program);
        glBindVertexArray(m_vao);

        glUniform2f(m_u_resolution, width, height);
        glUniform1f(m_u_time, glfwGetTime());
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

    if(!initialized && are_we_ready.valid() && are_we_ready.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
        are_we_ready.get();
        initialized = true;
        ThemeManager::apply_theme(config.settings.dark_mode ? ThemeType::Dark : ThemeType::Light);

        detection_future = std::async(std::launch::async, [this]() {
            return Detection::find_saves(config, d_result);
        });
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    Notify::render_notifications();

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    //ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_NoBackground;

    ImGui::Begin("Main Window", nullptr, window_flags);

    if(initialized) {
        if(last_dark_mode != config.settings.dark_mode) {
            ThemeManager::apply_theme(config.settings.dark_mode ? ThemeType::Dark : ThemeType::Light);
            last_dark_mode = config.settings.dark_mode;
        }
        render_ui();
    } 
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glUseProgram(0);
    glBindVertexArray(0);

    glfwSwapBuffers(window);
    glfwWaitEventsTimeout(1.0/60.0);
}

App::~App() {
    // int mx, my, mw, mh;
    // glfwGetMonitorWorkarea(glfwGetPrimaryMonitor(), &mx, &my, &mw, &mh); 
    //
    // if(config.win_props.x > mx && config.win_props.x < mx + mw) {
    //
    // }

    glfwGetWindowPos(window, &config.win_props.x, &config.win_props.y);
    glfwGetWindowSize(window, &config.win_props.width, &config.win_props.height);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
}
