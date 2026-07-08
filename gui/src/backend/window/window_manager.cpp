#include "window_manager.hpp"
#include <constants.hpp>

#include <backend/font_manager/font_manager.hpp>

#include <frontend/fonts/font_awesome.hpp>
#include <frontend/fonts/jbm_bold.h>
#include <frontend/fonts/jbm_med.h>
#include <frontend/fonts/jbm_reg.h>

std::pair<int, int> CWindowManager::get_size( ) {
    int width, height = 0;
    glfwGetFramebufferSize( m_window, &width, &height );
    return { width, height };
}

bool CWindowManager::should_continue( ) {
    bool window_open = glfwWindowShouldClose( m_window ) == 0;
#ifndef NDEBUG
    bool q_pressed = glfwGetKey( m_window, GLFW_KEY_Q ) == GLFW_PRESS;
    return window_open && !q_pressed;
#else
    return window_open;
#endif
}

void CWindowManager::run( std::function<void( )> pre, std::function<void( )> ui ) {
    do {
        pre( );
        ImGui_ImplOpenGL3_NewFrame( );
        ImGui_ImplGlfw_NewFrame( );
        ImGui::NewFrame( );

        ImGuiViewport* viewport = ImGui::GetMainViewport( );
        ImGui::SetNextWindowPos( viewport->Pos );
        ImGui::SetNextWindowSize( viewport->Size );

        ImGuiWindowFlags window_flags =
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground;

        ImGui::Begin( "Main Window", nullptr, window_flags );
        ui( );
        ImGui::End( );
        ImGui::Render( );

        ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData( ) );
        glfwSwapBuffers( m_window );
        glfwWaitEventsTimeout( 1.0 / 60.0 );
    } while ( should_continue( ) );
}

void CWindowManager::setup_opengl( ) {
    if ( !glfwInit( ) ) {
        throw std::runtime_error( "Failed to initialize GLFW" );
    }

    glfwWindowHint( GLFW_SAMPLES, 4 ); // 4x antialiasing (MSAA)
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE ); // no old OpenGL

    m_window = glfwCreateWindow( DEF_RES_W, DEF_RES_H, APP_NAME.data( ), nullptr, nullptr );
    if ( m_window == nullptr ) {
        glfwTerminate( );
        throw std::runtime_error( "Failed to create GLFW window, OpenGL 3.3 support is required" );
    }

    glfwSetWindowSizeLimits( m_window, MIN_RES_W, MIN_RES_H, MAX_RES_W, MAX_RES_H );
    glfwMakeContextCurrent( m_window );
    glfwSwapInterval( 1 );
    if ( !gladLoadGL( glfwGetProcAddress ) ) {
        throw std::runtime_error( "Failed to initialize GLAD" );
    }
}

void CWindowManager::setup_imgui( ) {
    ImGui::CreateContext( );

    ImGuiIO& io = ImGui::GetIO( );
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr; // no imgui.ini
    io.LogFilename = nullptr; // no imgui log pls

    // Regular fonts + merged icon font
    CFontManager::get( ).load_from_memory( { "jbm_reg", 20.0f, false, true }, (void*)jbm_reg, jbm_reg_len );
    CFontManager::get( ).load_from_memory(
        { "font_awesome", 18.0f, true, false }, (void*)font_awesome, font_awesome_len );

    CFontManager::get( ).load_from_memory( { "jbm_bold", 20.0f, false, false }, (void*)jbm_bold, jbm_bold_len );
    CFontManager::get( ).load_from_memory( { "jbm_med", 20.0f, false, false }, (void*)jbm_med, jbm_med_len );

    // Font variants - not sure if this is ideal since all thats different is the fontSize
    CFontManager::get( ).load_from_memory( { "jbm_header", 28.0f, false, false }, (void*)jbm_reg, jbm_reg_len );
    CFontManager::get( ).load_from_memory( { "jbm_title", 34.0f, false, false }, (void*)jbm_reg, jbm_reg_len );
    CFontManager::get( ).load_from_memory( { "jbm_small", 18.0f, false, false }, (void*)jbm_reg, jbm_reg_len );

    if ( !ImGui_ImplGlfw_InitForOpenGL( m_window, true ) ) {
        throw std::runtime_error( "Failed to initialize ImGui for OpenGL" );
    }
    if ( !ImGui_ImplOpenGL3_Init( ) ) {
        throw std::runtime_error( "Failed to initialize ImGui" );
    }
}
