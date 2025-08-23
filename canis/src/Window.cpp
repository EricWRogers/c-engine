#include "SDL3/SDL_video.h"
#include <Canis/Window.hpp>
#include <SDL3/SDL_log.h>
#include <cstdlib>
#include <SDL3/SDL_log.h>
//#include <SDL3/SDL_egl.h>
//#include <GL/glew.h>
#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <SDL3/SDL_opengl.h>
//#include <SDL3/SDL_opengl_glext.h>
//#include <SDL3/SDL_opengles.h>
#include <SDL3/SDL_opengles2_gl2.h>

// Simple vertex shader: passes position
static const char* vertexShaderSrc = R"(
attribute vec3 aPos;
void main() {
    gl_Position = vec4(aPos, 1.0);
}
)";

// Simple fragment shader: outputs red
static const char* fragmentShaderSrc = R"(
precision mediump float;
void main() {
    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
)";

namespace Canis {

Window::Window(const char* title, int width, int height) {
    if (SDL_Init(SDL_INIT_VIDEO) == false) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init Error: %s", SDL_GetError());
        std::exit(1);
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    m_window = SDL_CreateWindow(title,
                                width,
                                height,
                                SDL_WINDOW_OPENGL);
    if (!m_window) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create window: %s", SDL_GetError());
        std::exit(1);
    }

    m_context = SDL_GL_CreateContext(m_window);
    if (!m_context) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create GL context: %s", SDL_GetError());
        std::exit(1);
    }

    SDL_GL_SetSwapInterval(0);

    InitGL();
    SetupTriangle();
}

Window::~Window() {
    if (m_vbo)        glDeleteBuffers(1, &m_vbo);
    if (m_program)    glDeleteProgram(m_program);
    if (m_context)    SDL_GL_DestroyContext(m_context);
    if (m_window)     SDL_DestroyWindow(m_window);
    SDL_Quit();
}

void Window::PollEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            m_shouldClose = true;
        }
    }
}

bool Window::ShouldClose() const {
    return m_shouldClose;
}

void Window::Clear(float r, float g, float b, float a) const {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Window::Render() const {
    glUseProgram(m_program);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Window::Display() const {
    SDL_GL_SwapWindow(m_window);
}

void Window::InitGL() {
    // No additional GL initialization needed for GLES2
}

GLuint Window::CompileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Shader compile error: %s", log);
        std::exit(1);
    }
    return shader;
}

void Window::SetupTriangle() {
    // Compile shaders
    GLuint vert = CompileShader(GL_VERTEX_SHADER,   vertexShaderSrc);
    GLuint frag = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);

    m_program = glCreateProgram();
    glAttachShader(m_program, vert);
    glAttachShader(m_program, frag);
    glBindAttribLocation(m_program, 0, "aPos");
    glLinkProgram(m_program);

    GLint linked;
    glGetProgramiv(m_program, GL_LINK_STATUS, &linked);
    if (!linked) {
        char log[512];
        glGetProgramInfoLog(m_program, 512, nullptr, log);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Program link error: %s", log);
        std::exit(1);
    }

    glDeleteShader(vert);
    glDeleteShader(frag);

    // Vertex data for a triangle
    GLfloat vertices[] = {
         0.5f,  0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f,
         0.5f,  0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
    };

    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
}

} // namespace Canis

