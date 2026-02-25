#include <iostream>

#include "config/config.hpp"
#include "detection/detection.hpp"
#include "helpers/utils.hpp"
#include "command/command.hpp"
#include "core/ui/menu.hpp"
#include "core/ui/tabs/tabs.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <GL/gl.h>

#include "core/ui/fonts/jbm_reg.h"
#include "core/ui/fonts/jbm_med.h"
#include "core/ui/fonts/jbm_bold.h"
#include "core/globals.hpp"

int main() {
    if(!Config::config_exist()) {
        std::cerr << "Config is missing and could not be generated!\n";
        return 1;
    }

    auto result = Detection::find_saves();
    if(result.games.empty()) {
        std::cerr << "No savegames found!\n";
    }

    if(!glfwInit()) {
        std::cerr << "Failed to initialize GLFW.\n";
        return 1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // no old OpenGL

    GLFWwindow* window = glfwCreateWindow(1280, 720, "SaveManager", nullptr, nullptr);
    glfwSwapInterval(1);

    if(window == nullptr) {
        std::cerr << "Failed to create GLFW window. OpenGL 3.3 support is required!\n";
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    float title_fsize = 32.0f, gen_fsize = 20.0f, subt_fsize = 22.0f;
    ImFontConfig cfg_reg, cfg_med, cfg_bold;
    Fonts fonts;

    cfg_reg.FontDataOwnedByAtlas = false;
    cfg_med.FontDataOwnedByAtlas = false;
    cfg_bold.FontDataOwnedByAtlas = false;

    fonts.regular = io.Fonts->AddFontFromMemoryTTF((void*)jbm_reg, jbm_reg_len, gen_fsize, &cfg_reg);
    fonts.title = io.Fonts->AddFontFromMemoryTTF((void*)jbm_reg, jbm_reg_len, title_fsize, &cfg_reg);
    fonts.medium = io.Fonts->AddFontFromMemoryTTF((void*)jbm_med, jbm_med_len, gen_fsize, &cfg_med);
    fonts.bold = io.Fonts->AddFontFromMemoryTTF((void*)jbm_bold, jbm_bold_len, gen_fsize, &cfg_bold);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    bool show_demo_window = true;
    do{
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

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

        if(show_demo_window) {
            ImGui::ShowDemoWindow(&show_demo_window);
        } 

        ImGui::PushFont(fonts.title);
        ImGui::Text("SaveManager");
        ImGui::SameLine();
        ImGui::PopFont();
        ImGui::PushFont(fonts.medium);
        ImGui::Text(" | The definitive local save manager");
        ImGui::PopFont();

        ImGui::PushFont(fonts.regular);
        ImGui::Text("some text to fill the space");
        ImGui::PopFont();
        ImGui::Separator();

        ImGui::AlignTextToFramePadding();

        static ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_DrawSelectedOverline;
        if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags)) {
            if (ImGui::BeginTabItem("General"))  {
                Tabs::render_general_tab(fonts, result);
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

    // Menu main_menu("Save Manager");
    // main_menu.add_item("List games", handle_list);
    // main_menu.add_item("Backup", handle_backup);
    // main_menu.add_item("Restore", handle_restore);
    // main_menu.add_exit_item("Quit");
    //
    // while(main_menu.run(result)) {
    //
    // }
    return 0;
}
