#pragma once
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <backend/font_manager/font_manager.hpp>
#include <config/config.hpp>

class CWindowManager {
    public:
        explicit CWindowManager( CConfig& config ) : m_config( config ) {
            setup_opengl( );
            setup_imgui( );
        }
        ~CWindowManager( ) { cleanup( ); }

        void run( std::function<void( )> fun );
        void set_drop_callback( std::function<void( const std::vector<std::string>& )> );

    private:
        CConfig& m_config;
        GLFWwindow* m_window = nullptr;
        float m_content_scale = 1.0f;
        std::function<void( )> m_render_fn;

        ImGuiContext* m_imgui_ctx = nullptr;
        bool m_imgui_backend_init_gl3 = false;
        bool m_imgui_backend_init = false;
        void cleanup( );

        void setup_opengl( );
        void setup_imgui( );
        void apply_content_scale( float scale );
        bool should_continue( );
        void remember_window_size( );
        void render_frame( );

        void drop_callback( int count, const char** paths );
        std::function<void( const std::vector<std::string>& )> m_drop_fn;
};
