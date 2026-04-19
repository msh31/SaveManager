#include <backend/config/config.hpp>
#include "app/app.hpp"
#include <print>

int main(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if(std::string_view(argv[i]) == "--config-dir") {
            if(i + 1 >= argc) {
                std::println("you must provide a directory");
                return 1;
            }
            paths::set_config_dir(argv[i + 1]);
        }
    }

    App app;
    app.init();

    do{
        app.render();
    }
    while(
    glfwGetKey(app.window, GLFW_KEY_Q) != GLFW_PRESS &&
    glfwWindowShouldClose(app.window) == 0
);
    return 0;
}
