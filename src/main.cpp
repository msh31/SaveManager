#include <iostream>

#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

#include <sentinel/core/logger.h>

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

    do{
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    } while(
        glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
        glfwWindowShouldClose(window) == 0
    );

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
