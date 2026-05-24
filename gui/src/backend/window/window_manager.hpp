#pragma once
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <backend/font_manager/font_manager.hpp>

class CWindowManager {
    public:
        CWindowManager( ) {
            setup_opengl( );
            setup_imgui( );
        }
        ~CWindowManager( ) {
            if ( m_window != nullptr ) {
                ImGui_ImplOpenGL3_Shutdown( );
                ImGui_ImplGlfw_Shutdown( );
                ImGui::DestroyContext( );
                glfwDestroyWindow( m_window );
                glfwTerminate( );
            }
        }

        void run( std::function<void( )> fun );

    private:
        GLFWwindow*  m_window = nullptr;
        CFontManager m_font_mgr;

        void setup_opengl( );
        void setup_imgui( );
};
