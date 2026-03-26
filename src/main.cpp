#ifdef _WIN32
#include <windows.h>
#pragma comment(linker, "/subsystem:windows /entry:mainCRTStartup")
#endif

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "core/config/config.hpp"
#include "core/ui/notifications/notification.hpp"
#include "core/detection/detection.hpp"
#include "core/ui/themes/themes.hpp"
#include "core/globals.hpp"
#include "core/logger/logger.hpp"

#include "core/helpers/textures/textures.hpp"
#include "core/helpers/translations/translations.hpp"
#include "core/helpers/blacklist/blacklist.hpp"
#include "core/helpers/custom_games/custom_games.hpp"

#include "core/ui/tabs/settings/settings.hpp"
#include "core/ui/tabs/log/log.hpp"
#include "core/ui/tabs/transfer/transfer.hpp"
#include "core/ui/tabs/about/about.hpp"
#include "core/ui/tabs/general/general.hpp"
#include "core/ui/tabs/about/about.hpp"

#include "core/ui/fonts/jbm_reg.h"
#include "core/ui/fonts/jbm_med.h"
#include "core/ui/fonts/jbm_bold.h"

int main() {
    Config config;
    GeneralTab general_tab;
    TransferTab transfer_tab;
    LogTab log_tab;
    AboutTab about_tab;
    SettingsTab settings_tab;
    TabState state;

    if(!glfwInit()) {
        get_logger().error("Failed to initialize GLFW.");
        return 1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing (MSAA)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // no old OpenGL

    GLFWwindow* window = glfwCreateWindow(1600, 900, "SaveManager", nullptr, nullptr);
    if(window == nullptr) {
        get_logger().error("Failed to create GLFW window. OpenGL 3.3 support is required!");
        glfwTerminate();
        return 1;
    }
    glfwSetWindowSizeLimits(window, 1280, 720, 5120, 2880); //720p -> 5K, 16:9
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

    if(!config.init()) {
        get_logger().error("Config is missing and could not be generated!");
        Notify::show_notification("Config error!", "Config is missing and could not be generated!", 5000);
    }
    translations::init();
    Blacklist::init();
    CustomGamesFile::init();

    auto result = Detection::find_saves(config);
    if(result.games.empty()) {
        get_logger().warning("No savegames found!");
    }
    config.save();

    std::unordered_map<std::string, GLuint> game_textures;
    std::vector<std::future<Textures::ImageData>> texture_futures;
    int tex_w = 460, tex_h = 215;
    for (auto& game : result.games) {
        if(game.appid == "N/A") {
            continue;
        }
        texture_futures.push_back(std::async(std::launch::async, Textures::load_image, game.appid));
    }

    do{
        for (auto& texture : texture_futures) {
            if (texture.valid() && texture.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                Textures::ImageData data = texture.get(); 
                if(data.pixels.empty()) {
                    continue; 
                }

                game_textures[data.appid] = Textures::upload_image_to_gpu(data);
                // get_logger().info("Uploaded texture for: " + data.appid);
            }
        }

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

        if (ImGui::BeginTabBar("MyTabBar", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_DrawSelectedOverline)) {
            if (ImGui::BeginTabItem("General"))  {
                if (auto new_result = general_tab.render(fonts, result, game_textures, config, state)) {
                    result = *new_result;

                    for (auto it = game_textures.begin(); it != game_textures.end(); ++it) {
                        glDeleteTextures(1, &it->second);
                    }

                    game_textures = {};
                    texture_futures.clear();

                    for (auto& game : result.games) {
                        if(game.appid == "N/A") {
                            continue;
                        }

                        texture_futures.push_back(std::async(std::launch::async, Textures::load_image, game.appid));
                        // get_logger().info("Launched texture futures after refresh: " + std::to_string(texture_futures.size()));
                    }
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Transfer"))  {
                transfer_tab.render(fonts, result, config, state);
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

    if(!game_textures.empty()) {
        for (auto it = game_textures.begin(); it != game_textures.end(); ++it) {
            glDeleteTextures(1, &it->second);
        }
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
