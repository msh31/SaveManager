#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <unordered_map>

#include "core/network/network.hpp"
#include "core/config/config.hpp"
#include "core/ui/notifications/notification.hpp"
#include "core/detection/detection.hpp"
#include "core/helpers/textures.hpp"
#include "core/ui/tabs/tabs.hpp"
#include "core/ui/themes/themes.hpp"
#include "core/globals.hpp"
#include "core/logger/logger.hpp"

#include "core/ui/fonts/jbm_reg.h"
#include "core/ui/fonts/jbm_med.h"
#include "core/ui/fonts/jbm_bold.h"

int main() {
    Config config;
    if(!config.init()) {
        get_logger().error("Config is missing and could not be generated!");
        return 1;
    }

    auto result = Detection::find_saves(config);
    if(result.games.empty()) {
        get_logger().warning("No savegames found!");
    }
    config.save();
    get_logger().info("steam_path: " + config.settings.steam_path);
    get_logger().info("lutris_path: " + config.settings.lutris_path);
    get_logger().info("backup_path: " + config.settings.backup_path.string());

    if(!glfwInit()) {
        get_logger().error("Failed to initialize GLFW.");
        return 1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing (MSAA)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // no old OpenGL

    GLFWwindow* window = glfwCreateWindow(1600, 900, "SaveManager", nullptr, nullptr);
    glfwSetWindowSizeLimits(window, 1280, 720, 5120, 2880); //720p -> 5K, 16:9

    if(window == nullptr) {
        get_logger().error("Failed to create GLFW window. OpenGL 3.3 support is required!");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        get_logger().error("Failed to initialize GLAD");
        return 1;
    }
    ImGui::CreateContext();
    ThemeManager::apply_theme(ThemeType::Dark);

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    float title_fsize = 34.0f, gen_fsize = 20.0f, subt_fsize = 22.0f, head_fsize = 28.0f;
    ImFontConfig cfg_reg, cfg_med, cfg_bold, cfg_head;
    Fonts fonts;

    cfg_reg.FontDataOwnedByAtlas = false;
    cfg_med.FontDataOwnedByAtlas = false;
    cfg_bold.FontDataOwnedByAtlas = false;
    cfg_head.FontDataOwnedByAtlas = false;

    fonts.regular = io.Fonts->AddFontFromMemoryTTF((void*)jbm_reg, jbm_reg_len, gen_fsize, &cfg_reg);
    fonts.title = io.Fonts->AddFontFromMemoryTTF((void*)jbm_reg, jbm_reg_len, title_fsize, &cfg_reg);
    fonts.medium = io.Fonts->AddFontFromMemoryTTF((void*)jbm_med, jbm_med_len, gen_fsize, &cfg_med);
    fonts.bold = io.Fonts->AddFontFromMemoryTTF((void*)jbm_bold, jbm_bold_len, gen_fsize, &cfg_bold);
    fonts.header = io.Fonts->AddFontFromMemoryTTF((void*)jbm_reg, jbm_reg_len, head_fsize, &cfg_head);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    GLuint game_texture; 
    std::unordered_map<std::string, GLuint> game_textures;
    int tex_w = 460, tex_h = 215;
    for (auto& game : result.games) {
        if(game.appid == "N/A") {
            continue;
        }

        if(!Network::download_game_image(game.appid)) {
            continue;
        }
        fs::path path;

        path = cache_dir / (game.appid + ".jpg");
        LoadTextureFromFile(path.string().c_str(), &game_texture, &tex_w, &tex_h);

        game_textures[game.appid] = game_texture;
    }

    do{
        glClear(GL_COLOR_BUFFER_BIT);

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

        ImGui::PushFont(fonts.title);
        ImGui::Text("SaveManager");
        ImGui::PopFont();
        ImGui::Separator();

        ImGui::AlignTextToFramePadding();

        static ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_DrawSelectedOverline;
        if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags)) {
            if (ImGui::BeginTabItem("General"))  {
                Tabs::render_general_tab(fonts, result, game_textures, config);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Log"))  {
                Tabs::render_log_tab(fonts);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("About"))  {
                Tabs::render_about_tab(fonts);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Settings"))  {
                Tabs::render_settings_tab(fonts, config);
                ImGui::EndTabItem();
            }
            // if (ImGui::BeginTabItem("Debug"))  {
            //     Tabs::render_debug_tab(fonts);
            //     ImGui::EndTabItem();
            // }

            ImGui::EndTabBar();
        }
        ImGui::Separator();

        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    while(
glfwGetKey(window, GLFW_KEY_Q) != GLFW_PRESS &&
glfwWindowShouldClose(window) == 0
);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
