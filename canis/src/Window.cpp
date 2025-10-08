#include "SDL3/SDL_video.h"
#include <Canis/Window.hpp>
#include <Canis/OpenGL.hpp>
#include <SDL3/SDL_log.h>
#include <cstdlib>

namespace Canis {

Window::Window(const char* title, int width, int height) {
    if (SDL_Init(SDL_INIT_VIDEO) == false) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init Error: %s", SDL_GetError());
        std::exit(1);
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    m_screenWidth = width;
    m_screenHeight = height;

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
}

Window::~Window() {
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

void Window::Display() const {
    SDL_GL_SwapWindow(m_window);
}

void Window::InitGL() {
    // No additional GL initialization needed for GLES2
}

} // namespace Canis

