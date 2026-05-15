#include <config/config.hpp>
#include <app/app.hpp>
#include <utils/paths.hpp>
#include <logger/logger.hpp>
#include <print>

int main(int argc, char *argv[]) {
    bool redirected = false;

    for (int i = 1; i < argc; i++) {
        if(std::string_view(argv[i]) == "--config-dir") {
            if(i + 1 >= argc) {
                std::println("you must provide a directory");
                return 1;
            }
            paths::set_config_dir(argv[i + 1]);
            if(!fs::exists(paths::redirect_file())) {
                fs::create_directories(paths::default_config_dir());
            }
            std::ofstream cf (paths::redirect_file());
            cf << argv[i + 1];
            cf.close();
            redirected = true;
        }
    }

    if(!redirected && fs::exists(paths::redirect_file())) {
        std::ifstream redirect_file(paths::redirect_file());
        if(!redirect_file.is_open()) {
            std::println("failed to open redirect file for reading");
            return -1;
        } 
        std::string r_path;
        while(std::getline(redirect_file, r_path)) {
            paths::set_config_dir(r_path);
        }
        redirect_file.close();
        redirected = true;
    }

    init_logger("[%n]: [%l] %d-%m-%Y %H:%M:%S - %v");

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
