#pragma once
#include "core/config/config.hpp"
#include "core/ui/tabs/settings/settings.hpp"
#include "core/ui/tabs/log/log.hpp"
#include "core/ui/tabs/transfer/transfer.hpp"
#include "core/ui/tabs/about/about.hpp"
#include "core/ui/tabs/general/general.hpp"
#include "core/ui/tabs/about/about.hpp"

#include "core/helpers/textures/textures.hpp"

class App {
public:
    ~App();

    void init();
    void render();

    GLFWwindow* window = nullptr;
    std::future<void> are_we_ready;
private:
    bool setup_opengl();
    bool setup_imgui();

    bool initialized = false;
    bool last_dark_mode = true;

    void render_ui();
    void render_loading_screen();

    Fonts fonts;

    Config config;
    GeneralTab general_tab;
    TransferTab transfer_tab;
    LogTab log_tab;
    AboutTab about_tab;
    SettingsTab settings_tab;
    TabState state;

    std::unordered_map<std::string, GLuint> game_textures;
    std::vector<std::future<Textures::ImageData>> texture_futures;

    Detection::DetectionResult d_result;
};
