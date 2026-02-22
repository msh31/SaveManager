#include <iostream>

#include "config/config.hpp"
#include "detection/detection.hpp"
#include "helpers/utils.hpp"
#include "command/command.hpp"
#include "core/ui/menu.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <GL/gl.h>

#include "core/ui/fonts/jbm_reg.h"
#include "core/ui/fonts/jbm_med.h"
#include "core/ui/fonts/jbm_bold.h"

int main() {
    if(!Config::config_exist()) {
        std::cerr << "Config is missing and could not be generated!\n";
        return 1;
    }

    auto result = Detection::find_saves();
    if(result.games.empty()) {
        std::cerr << "No savegames found, exiting..\n";
        return 1;
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
    ImFontConfig cfg_reg;
    cfg_reg.FontDataOwnedByAtlas = false;
    ImFont* font_reg = io.Fonts->AddFontFromMemoryTTF((void*)jbm_reg, jbm_reg_len, gen_fsize, &cfg_reg);
    ImFont* font_title = io.Fonts->AddFontFromMemoryTTF((void*)jbm_reg, jbm_reg_len, title_fsize, &cfg_reg);

    ImFontConfig cfg_med;
    cfg_med.FontDataOwnedByAtlas = false;
    ImFont* font_med = io.Fonts->AddFontFromMemoryTTF((void*)jbm_med, jbm_med_len, gen_fsize, &cfg_med);

    ImFontConfig cfg_bold;
    cfg_bold.FontDataOwnedByAtlas = false;
    ImFont* font_bold = io.Fonts->AddFontFromMemoryTTF((void*)jbm_bold, jbm_bold_len, gen_fsize, &cfg_bold);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

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

        float panelWidth = ImGui::GetContentRegionAvail().x * 1.0f;
        float panelHeight = ImGui::GetContentRegionAvail().y * 1.0f;

        ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - panelWidth) * 0.5f);
        ImGui::SetCursorPosY((ImGui::GetContentRegionAvail().y - panelHeight) * 0.5f);

        ImGui::BeginChild("HostSetupPanel", ImVec2(panelWidth, panelHeight), true, ImGuiChildFlags_AlwaysUseWindowPadding);

        ImGui::PushFont(font_title);
        ImGui::Text("SaveManager");
        ImGui::SameLine();
        ImGui::PopFont();
        ImGui::PushFont(font_med);
        ImGui::Text(" | The definitive local save manager");
        ImGui::PopFont();

        ImGui::PushFont(font_med);
        ImGui::Text("some text to fill the space");
        ImGui::PopFont();
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 15));
        ImGui::EndChild();

        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    while(
        glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
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
