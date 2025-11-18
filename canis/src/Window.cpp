#include <Canis/Window.hpp>
#include <Canis/OpenGL.hpp>
#include <Canis/Debug.hpp>
#include <SDL3/SDL.h>
#include <SDL3/SDL_log.h>
#include <cstdlib>

#include <stb_image.h>

namespace Canis
{
    Window::Window(const char *title, int width, int height)
    {
        // if linux
#ifdef __linux__
        SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "x11");
#endif
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMEPAD) == false)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init Error: %s", SDL_GetError());
            std::exit(1);
        }

#ifdef __EMSCRIPTEN__
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
        SDL_SetHint(SDL_HINT_RENDER_GPU_LOW_POWER, "0"); // prefer high-perf GPU
#endif

        m_screenWidth = width;
        m_screenHeight = height;

        m_window = SDL_CreateWindow(title,
                                    width,
                                    height,
                                    SDL_WINDOW_OPENGL);
        if (!m_window)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create window: %s", SDL_GetError());
            std::exit(1);
        }

        m_context = (void*)SDL_GL_CreateContext((SDL_Window*)m_window);
        if (!m_context)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create GL context: %s", SDL_GetError());
            std::exit(1);
        }

        

        InitGL();

        SDL_GL_SetSwapInterval(0);
    }

    Window::~Window()
    {
        if (m_context)
            SDL_GL_DestroyContext((SDL_GLContext)m_context);
        if (m_window)
            SDL_DestroyWindow((SDL_Window*)m_window);
        SDL_Quit();
    }

    void Window::Clear() const
    {
        float r = m_clearColor.r;
        float g = m_clearColor.g;
        float b = m_clearColor.b;
        float a = m_clearColor.a;
        glClearColor(r, g, b, a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void Window::SetClearColor(Color _color)
    {
        m_clearColor = _color;
    }

    void Window::SwapBuffer() const
    {
        SDL_GL_SwapWindow((SDL_Window*)m_window);
    }

    void Window::SetWindowIcon(std::string _path)
    {
        stbi_set_flip_vertically_on_load(false);

        int w, h, channels;
        // force RGBA
        unsigned char* pixels = stbi_load(_path.c_str(), &w, &h, &channels, 4);
        if (!pixels)
        {
            Debug::Log("Failed to load icon '%s': %s", _path.c_str(), stbi_failure_reason());
            return;
        }

        SDL_Surface* surface = SDL_CreateSurfaceFrom(
            w,
            h,
            SDL_PIXELFORMAT_RGBA32,
            pixels,
            w * 4
        );

        if (!surface)
        {
            SDL_Log("Failed to create icon surface: %s", SDL_GetError());
            stbi_image_free(pixels);
            return;
        }
        
        SDL_SetWindowIcon((SDL_Window*)m_window, surface);

        SDL_DestroySurface(surface);
        stbi_image_free(pixels);
    }

    void Window::InitGL()
    {
#ifdef __EMSCRIPTEN__

#else
        SDL_GL_MakeCurrent((SDL_Window*)m_window, (SDL_GLContext)m_context);
        
        glewExperimental = GL_TRUE; // required for core profile
        GLenum err = glewInit();
        if (err != GLEW_OK)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                        "GLEW initialization failed: %s",
                        (const char*)glewGetErrorString(err));
            std::exit(1);
        }
#endif
    }

    void Window::SetWindowSize(int _width, int _height)
    {
        if (_width == m_screenWidth && _height == m_screenHeight)
            return;

        m_resized = true;

        m_screenWidth = _width;
        m_screenHeight = _height;
    }

    void Window::SetResized(bool _resized)
    {
        m_resized = _resized;
    }

    bool Window::IsResized()
    {
        return m_resized;
    }

    Window::Sync Window::GetSync()
    {
        int sync;
        SDL_GL_GetSwapInterval(&sync);
        return (Sync)sync;
    }

    void Window::SetSync(Sync _type)
    {
        SDL_GL_SetSwapInterval(_type);
    }
} // namespace Canis