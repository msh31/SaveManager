#include "detection/games.hpp"
#include "includes.hpp"
#include "theme_manager/theme_manager.hpp"
#include "ui_manager/ui_manager.hpp"
#include "fonts/rubik.h"

int main() {
    logger log;
    Config config; //auto calls load, destructor ccalls save :)
    UIManager uiManager(config);

    log.fileLoggingEnabled = false;

    if (game_detection::loadGameDatabase("./data/games.json")) {
        log.success("Game database loaded successfully");
    } else {
        log.error("Failed to load game database");
    }

    // game_detection::scanSteamLibrary("/mnt/games/", const std::string &profileID)

    if(!glfwInit()) {
        log.error("Failed to initialize GLFW.");
        return -1;
    }

    log.success("wassup world");

    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // no old OpenGL

    GLFWwindow* window;
    window = glfwCreateWindow(1250, 750, "SaveManager", NULL, NULL);

    if(window == NULL) {
        log.error("Failed to create GLFW window. OpenGL 3.3 support is required!");
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

    ImFontConfig cfg16;
    cfg16.FontDataOwnedByAtlas = false;
    ImFont* font16 = io.Fonts->AddFontFromMemoryTTF((void*)Rubik, Rubik_len, 16.0f, &cfg16);

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

        uiManager.Render(window_flags);

        bool showDemoWindow = true;
        ImGui::ShowDemoWindow(&showDemoWindow);

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
    log.success("bye world");
    return 0;
}
