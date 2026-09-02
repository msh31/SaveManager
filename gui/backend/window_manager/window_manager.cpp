#include "window_manager.hpp"
#include <branding.hpp>

#include <frontend/fonts/font_registry.hpp>
#include <frontend/theme/theme.hpp>

#include <logger.hpp>

#if defined( _WIN32 )
    #define GLFW_EXPOSE_NATIVE_WIN32
    #include <GLFW/glfw3native.h>
    #include <dwmapi.h>

    #ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
        #define DWMWA_USE_IMMERSIVE_DARK_MODE 20
    #endif
#endif

namespace {
    constexpr int kMinWindowW = 640;
    constexpr int kMinWindowH = 480;
    constexpr float kDefaultWindowFraction = 0.7f; // of the primary monitor's work area
    static bool callback_error_triggered = false;
} // namespace

bool CWindowManager::should_continue( ) {
    bool window_open = glfwWindowShouldClose( m_window ) == 0;
#ifndef NDEBUG
    bool q_pressed = glfwGetKey( m_window, GLFW_KEY_Q ) == GLFW_PRESS;
    return window_open && !q_pressed;
#else
    return window_open;
#endif
}

void CWindowManager::run( std::function<void( )> fun ) {
    m_render_fn = fun;

    do {
        render_frame( );
        glfwWaitEventsTimeout( 1.0 / 60.0 );
    } while ( should_continue( ) );

    remember_window_size( );
}

void CWindowManager::render_frame( ) {
    if ( !m_render_fn ) return;

    glClear( GL_COLOR_BUFFER_BIT );

    ImGui_ImplOpenGL3_NewFrame( );
    ImGui_ImplGlfw_NewFrame( );
    ImGui::NewFrame( );

    ImGuiViewport* viewport = ImGui::GetMainViewport( );
    ImGui::SetNextWindowPos( viewport->Pos );
    ImGui::SetNextWindowSize( viewport->Size );
    // ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus |
                                    ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoScrollbar |
                                    ImGuiWindowFlags_NoScrollWithMouse;

    if ( m_config.settings.use_bg ) {
        window_flags |= ImGuiWindowFlags_NoBackground;
    }

    ImGui::Begin( "Main Window", nullptr, window_flags );
    m_render_fn( );
    ImGui::End( );
    ImGui::Render( );

    ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData( ) );
    glfwSwapBuffers( m_window );
}

static void error_callback( int error, const char* description ) {
    auto str = std::format( "A fatal error occured: {}", description );
    SPDLOG_CRITICAL( "{}", str );
    callback_error_triggered = true;
}

void CWindowManager::cleanup() {
    if ( m_imgui_backend_init_gl3 ) {
        ImGui_ImplOpenGL3_Shutdown( );
    }
    if ( m_imgui_backend_init ) {
        ImGui_ImplGlfw_Shutdown( );
    }
    if ( m_imgui_ctx ) {
        ImGui::DestroyContext( );
    }
    if ( m_window ) {
        glfwTerminate( );
    }
}

void CWindowManager::setup_opengl( ) {
    SPDLOG_INFO( "Setting up OpenGL.." );
    glfwSetErrorCallback( error_callback );
    if ( !glfwInit( ) ) {
        throw std::runtime_error( "Failed to initialize GLFW" );
    }

    // OpenGL 3.3 Core
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

    int width = m_config.settings.window_w;
    int height = m_config.settings.window_h;
    if ( width <= 0 || height <= 0 ) {
        // no persisted size yet - derive a sane default from the primary monitor instead of a hardcoded resolution
        width = kMinWindowW;
        height = kMinWindowH;
        if ( GLFWmonitor* monitor = glfwGetPrimaryMonitor( ) ) {
            int mon_x, mon_y, mon_w, mon_h;
            glfwGetMonitorWorkarea( monitor, &mon_x, &mon_y, &mon_w, &mon_h );
            width = std::max( kMinWindowW, static_cast<int>( mon_w * kDefaultWindowFraction ) );
            height = std::max( kMinWindowH, static_cast<int>( mon_h * kDefaultWindowFraction ) );
        }
    }

    callback_error_triggered = false;
    m_window = glfwCreateWindow( width, height, APP_NAME, nullptr, nullptr );
    if ( m_window == nullptr || callback_error_triggered ) {
        glfwTerminate( );
        throw std::runtime_error( "[WindowManager] Failed to create window!" );
    }

#if defined( _WIN32 )
    BOOL dark_mode = m_config.settings.dark_mode;
    DwmSetWindowAttribute(
        glfwGetWin32Window( m_window ), DWMWA_USE_IMMERSIVE_DARK_MODE, &dark_mode, sizeof( dark_mode ) );
#endif

    glfwSetWindowSizeLimits( m_window, kMinWindowW, kMinWindowH, GLFW_DONT_CARE, GLFW_DONT_CARE );
    glfwMakeContextCurrent( m_window );
    glfwSwapInterval( 1 ); // vsync
    if ( !gladLoadGL( glfwGetProcAddress ) ) {
        cleanup( );
        throw std::runtime_error( "Failed to initialize GLAD!" );
    }
}

void CWindowManager::remember_window_size( ) {
    int w = 0, h = 0;
    glfwGetWindowSize( m_window, &w, &h );
    m_config.settings.window_w = w;
    m_config.settings.window_h = h;
}

void CWindowManager::apply_content_scale( float scale ) {
#if defined( __APPLE__ )
    // IMPORTANT: macOS resizes teh framebuffer than the window like macOS causing double scaling
    return;
#endif
    m_content_scale = scale;
    ThemeManager::apply_scale( scale );
    ImGui::GetStyle( ).FontScaleDpi = scale;
}

void CWindowManager::set_drop_callback(std::function<void(const std::vector<std::string>&)> fn ) {
    m_drop_fn = fn;
}

void CWindowManager::drop_callback( int count, const char** paths ) {
    std::vector<std::string> data = { };
    for ( int i = 0; i < count; i++ ) {
        data.emplace_back( paths[i] );
    }
    if ( m_drop_fn) m_drop_fn( data );
}

void CWindowManager::setup_imgui( ) {
    SPDLOG_INFO( "Setting up ImGui.." );
    m_imgui_ctx = ImGui::CreateContext( );

    ImGuiIO& io = ImGui::GetIO( );
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr; // no imgui.ini
    io.LogFilename = nullptr; // no imgui log pls

    ThemeManager::apply_style( );
    CFontManager::get( ).load_all( font_registry( ) );

    glfwSetWindowUserPointer( m_window, this );
    glfwSetWindowContentScaleCallback( m_window, []( GLFWwindow* window, float xscale, float ) {
        static_cast<CWindowManager*>( glfwGetWindowUserPointer( window ) )->apply_content_scale( xscale );
    } );

    glfwSetWindowRefreshCallback( m_window, []( GLFWwindow* window ) {
        static_cast<CWindowManager*>( glfwGetWindowUserPointer( window ) )->render_frame( );
    } );

    glfwSetDropCallback( m_window, []( GLFWwindow* window, int count, const char** paths ) {
        static_cast<CWindowManager*>( glfwGetWindowUserPointer( window ) )->drop_callback( count, paths );
    } );

    float xscale = 1.0f, yscale = 1.0f;
    glfwGetWindowContentScale( m_window, &xscale, &yscale );
    apply_content_scale( xscale );

    m_imgui_backend_init = ImGui_ImplGlfw_InitForOpenGL( m_window, true );
    if ( !m_imgui_backend_init ) {
        cleanup( );
        throw std::runtime_error( "Failed to initialize ImGui for OpenGL" );
    }
    m_imgui_backend_init_gl3 = ImGui_ImplOpenGL3_Init( );
    if ( !m_imgui_backend_init_gl3 ) {
        cleanup( );
        throw std::runtime_error( "Failed to initialize ImGui" );
    }
    ThemeManager::apply_colors( m_config.settings.dark_mode ? ThemeType::Dark : ThemeType::Light );
}
