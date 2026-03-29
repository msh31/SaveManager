#include "app/app.hpp"

int main() {
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
