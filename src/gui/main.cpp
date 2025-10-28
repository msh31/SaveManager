#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"
#include <sentinel/core/logger.h>

//#include "../core/savemanager.hpp"
#include "theme_manager/theme_manager.hpp"

int main() {
    logger logger;
    logger.fileLoggingEnabled = false;

    if(!glfwInit()) {
        logger.error("Failed to initialize GLFW.");
        return -1;
    }

    logger.success("wassup world");

    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // no old OpenGL

    GLFWwindow* window;
    window = glfwCreateWindow(750, 550, "SaveManager", NULL, NULL);

    if(window == NULL) {
        logger.error("Failed to create GLFW window. OpenGL 3.3 support is required!");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ThemeManager::ApplyTheme(ThemeType::Dark);

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

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
        ImGui::Text("SaveManager");
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 15));
        ImGui::Text("Another piece of text");

        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        glfwPollEvents();
    } while(
        glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
        glfwWindowShouldClose(window) == 0
    );

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    logger.success("bye world");
    return 0;
}
