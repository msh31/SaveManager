#include "shader.hpp"

CShader::CShader( ) {
    GLuint vert = compile_shader( default_vert, GL_VERTEX_SHADER );
    GLuint frag = compile_shader( default_frag, GL_FRAGMENT_SHADER );
    m_shader_program = link_program( vert, frag );
    m_u_resolution = glGetUniformLocation( m_shader_program, "iResolution" );
    m_u_time = glGetUniformLocation( m_shader_program, "iTime" );
    init_quad( );
}

CShader::~CShader( ) {
    glDeleteVertexArrays( 1, &m_vao );
    glDeleteBuffers( 1, &m_vbo );
    glDeleteProgram( m_shader_program );
}

void CShader::render( int w, int h ) {
    glUseProgram( m_shader_program );
    glBindVertexArray( m_vao );

    glUniform2f( m_u_resolution, w, h );
    glUniform1f( m_u_time, glfwGetTime( ) );
    glDrawArrays( GL_TRIANGLES, 0, 6 );
    glBindVertexArray( 0 );
}

GLuint CShader::compile_shader( const char* source, GLenum type ) {
    GLuint shader_id = glCreateShader( type );
    glShaderSource( shader_id, 1, &source, NULL );
    glCompileShader( shader_id );

    GLint is_compiled = 0;
    glGetShaderiv( shader_id, GL_COMPILE_STATUS, &is_compiled );
    if ( is_compiled == GL_FALSE ) {
        GLint log_length = 0;
        glGetShaderiv( shader_id, GL_INFO_LOG_LENGTH, &log_length );
        std::string logs( log_length, '\0' );
        glGetShaderInfoLog( shader_id, log_length, NULL, logs.data( ) );
        SPDLOG_WARN( "Shader compilation error: {}", logs );
        glDeleteShader( shader_id );
        return GL_FALSE;
    }
    return shader_id;
}

GLuint CShader::link_program( GLuint vert, GLuint frag ) {
    GLuint pid = glCreateProgram( );
    GLint is_linked = 0;

    glAttachShader( pid, vert );
    glAttachShader( pid, frag );

    glLinkProgram( pid );

    glGetProgramiv( pid, GL_LINK_STATUS, &is_linked );
    if ( is_linked == GL_FALSE ) {
        GLint log_length = 0;
        glGetProgramiv( pid, GL_INFO_LOG_LENGTH, &log_length );
        std::string logs( log_length, '\0' );
        glGetProgramInfoLog( pid, log_length, NULL, logs.data( ) );
        SPDLOG_WARN( "Shader linker error: {}", logs );

        glDeleteProgram( pid );

        glDeleteShader( vert );
        glDeleteShader( frag );
        return GL_FALSE;
    }

    glDetachShader( pid, vert );
    glDetachShader( pid, frag );

    glDeleteShader( vert );
    glDeleteShader( frag );
    return pid;
}

void CShader::init_quad( ) {
    float vertices[12] = {
        -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f,
    };

    glGenVertexArrays( 1, &m_vao );
    glGenBuffers( 1, &m_vbo );

    glBindVertexArray( m_vao );
    glBindBuffer( GL_ARRAY_BUFFER, m_vbo );
    glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices, GL_STATIC_DRAW );

    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof( float ), (void*)0 );
    glEnableVertexAttribArray( 0 );

    glBindVertexArray( 0 );
}
