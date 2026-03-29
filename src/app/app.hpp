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
private:
    bool setup_opengl();
    bool setup_imgui();

    void render_ui();
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
