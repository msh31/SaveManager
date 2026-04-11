#ifdef _WIN32
#include <windows.h>
#pragma comment(linker, "/subsystem:windows /entry:mainCRTStartup")
#endif

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "app.hpp"
#include "globals.hpp"
#include "backend/logger/logger.hpp"

#include "frontend/ui/themes/themes.hpp"
#include "frontend/ui/notifications/notification.hpp"

#include "frontend/ui/fonts/jbm_reg.h"
#include "frontend/ui/fonts/jbm_med.h"
#include "frontend/ui/fonts/jbm_bold.h"

#include "backend/detection/detection.hpp"
#include "backend/utils/translations/translations.hpp"
#include "backend/utils/blacklist/blacklist.hpp"
#include "backend/utils/custom_games/custom_games.hpp"

void App::init() {
    if(!setup_opengl()) {
        return;
    }
    if(!setup_imgui()) {
        return;
    }

    are_we_ready = std::async(std::launch::async, [this]() {
        if(!config.init()) {
            get_logger().error("Config is missing and could not be generated!");
            Notify::show_notification("Config error!", "Config is missing and could not be generated!", 5000);
        }

        translations::init();
        Blacklist::init();
        CustomGamesFile::init();

        d_result = Detection::find_saves(config);
        if(d_result.games.empty()) {
            get_logger().warning("No savegames found!");
        }
        config.save();
    });
}

void App::render_ui() {
    ImGui::PushFont(fonts.title);
    ImGui::Text(APP_NAME);
    ImGui::PopFont();
    ImGui::Separator();

    ImGui::AlignTextToFramePadding();

    if (ImGui::BeginTabBar("MyTabBar", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_DrawSelectedOverline)) {
        if (ImGui::BeginTabItem("General"))  {
            if (auto new_d_result = general_tab.render(fonts, d_result, game_textures, config, state)) {
                d_result = *new_d_result;

                for (auto& [appid, texture] : game_textures) glDeleteTextures(1, &texture);
                game_textures = {};
                texture_futures.clear();

                for (auto& game : d_result.games) {
                    if(game.appid == "N/A") continue;
                    texture_futures.push_back(std::async(std::launch::async, Textures::load_image, game.appid));
                }
            }
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
            settings_tab.m_refresh_requested = &refresh_requested;
            settings_tab.render(fonts, config);
            ImGui::EndTabItem();
        }
        // if (ImGui::BeginTabItem("Debug"))  {
        //     Tabs::render_debug_tab(fonts);
        //     ImGui::EndTabItem();
        // }

        ImGui::EndTabBar();
    }
    ImGui::Separator();
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

bool App::setup_opengl() {
    if(!glfwInit()) {
        get_logger().error("Failed to initialize GLFW.");
        return false;
    }

    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing (MSAA)
                   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // no old OpenGL

    window = glfwCreateWindow(DEF_RES_W, DEF_RES_H, APP_NAME, nullptr, nullptr);
    if(window == nullptr) {
        get_logger().error("Failed to create GLFW window. OpenGL 3.3 support is required!");
        glfwTerminate();
        return false;
    }

    glfwSetWindowSizeLimits(window, MIN_RES_W, MIN_RES_H, MAX_RES_W, MAX_RES_H); 
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    if (!gladLoadGL(glfwGetProcAddress)) {
        get_logger().error("Failed to initialize GLAD");
        return false;
    }

    return true;
}

bool App::setup_imgui() {
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImFontConfig cfg_reg, cfg_med, cfg_small, cfg_bold, cfg_head, cfg_title;
    cfg_reg.FontDataOwnedByAtlas = false;
    cfg_med.FontDataOwnedByAtlas = false;
    cfg_small.FontDataOwnedByAtlas = false;
    cfg_bold.FontDataOwnedByAtlas = false;
    cfg_head.FontDataOwnedByAtlas = false;
    cfg_title.FontDataOwnedByAtlas = false;

    fonts.regular = io.Fonts->AddFontFromMemoryTTF((void*)jbm_reg, jbm_reg_len, 20.0f, &cfg_reg);
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
    glClear(GL_COLOR_BUFFER_BIT);

    if(refresh_requested) {
        for (auto& [appid, texture] : game_textures) glDeleteTextures(1, &texture);
        game_textures.clear();
        texture_futures.clear();

        for (auto& game : d_result.games) {
            if(game.appid == "N/A") {
                continue;
            }

            texture_futures.push_back(std::async(std::launch::async, Textures::load_image, game.appid));
            // get_logger().info("Launched texture futures after refresh: " + std::to_string(texture_futures.size()));
        }
        refresh_requested = false;
        // get_logger().debug("refresh request completed!");
    }

    if(!initialized && are_we_ready.valid() && are_we_ready.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
        are_we_ready.get();
        initialized = true;
        ThemeManager::apply_theme(config.settings.dark_mode ? ThemeType::Dark : ThemeType::Light);
    }

    if(initialized && texture_futures.empty()) {
        for (auto& game : d_result.games) {
            if(game.appid == "N/A") continue;
            texture_futures.push_back(std::async(std::launch::async, Textures::load_image, game.appid));
        }
    }

    for (auto& texture : texture_futures) {
        if (texture.valid() && texture.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            Textures::ImageData data = texture.get(); 
            if(data.pixels.empty()) continue;
            game_textures[data.appid] = Textures::upload_image_to_gpu(data);
        }
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
        ImGuiWindowFlags_NoNavFocus;

    ImGui::Begin("Main Window", nullptr, window_flags);

    if(initialized) {
        if(last_dark_mode != config.settings.dark_mode) {
            ThemeManager::apply_theme(config.settings.dark_mode ? ThemeType::Dark : ThemeType::Light);
            last_dark_mode = config.settings.dark_mode;
        }
        render_ui();
    } else {
        render_loading_screen();
    }

    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
    glfwPollEvents();
}

App::~App() {
    if(!game_textures.empty()) for (auto& [appid, texture] : game_textures) glDeleteTextures(1, &texture);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
}
