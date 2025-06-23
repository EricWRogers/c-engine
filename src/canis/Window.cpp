#include "Canis.hpp"
#include "Window.hpp"
#include "External/OpenGL.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

namespace Canis
{
    int Window::Create(std::string _windowName, int _screenWidth, int _screenHeight, unsigned int _currentFlags)
    {
        CanisData& data = GetCanisData();
        // if you wanted you application to support multiple rendering apis 
        // you would not want to hard code it here
        Uint32 flags = SDL_WINDOW_OPENGL;

        data.screenWidth = _screenWidth;
        data.screenHeight = _screenHeight;

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
        #endif
        
        if (_currentFlags & WindowFlags::FULLSCREEN)
        {
            flags |= SDL_WINDOW_FULLSCREEN;
            data.fullscreen = true;
        }

        if (_currentFlags & WindowFlags::BORDERLESS) {
            flags |= SDL_WINDOW_BORDERLESS;
        }

        if (_currentFlags & WindowFlags::RESIZEABLE) {
            flags |= SDL_WINDOW_RESIZABLE;
        }

            //flags |= SDL_WINDOW_RESIZABLE;

        #ifdef __EMSCRIPTEN__
        // Create Window
        m_sdlWindow = SDL_CreateWindow(_windowName.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _screenWidth, _screenHeight, flags);
        #else
        // Create Window
        data.sdlWindow = SDL_CreateWindow(_windowName.c_str(), _screenWidth, _screenHeight, flags);
        
        SDL_Surface *surface;     // Declare an SDL_Surface to be filled in with pixel data from an image file
        Uint16 pixels[16*16] = {  // ...or with raw pixel data:
            0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0xf435, 0xf435, 0xf435, 0xf435, 0xf435, 0xf435, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff,
            0x0fff, 0x0fff, 0x0fff, 0xf435, 0xf435, 0xf435, 0xf435, 0xffff, 0xffff, 0xf435, 0xf435, 0xf435, 0xf435, 0x0fff, 0x0fff, 0x0fff,
            0x0fff, 0x0fff, 0xf435, 0xf435, 0xf435, 0xf435, 0xf435, 0xffff, 0xffff, 0xf435, 0xf435, 0xf435, 0xf435, 0xf435, 0x0fff, 0x0fff,
            0x0fff, 0xf435, 0xf435, 0xf435, 0xf435, 0xf435, 0xffff, 0xffff, 0xffff, 0xffff, 0xf435, 0xf435, 0xf435, 0xf435, 0xf435, 0x0fff,
            0x0fff, 0xf435, 0xffff, 0xffff, 0xf435, 0xf435, 0xffff, 0xffff, 0xffff, 0xffff, 0xf435, 0xf435, 0xf435, 0xf435, 0xf435, 0x0fff,
            0xf435, 0xf435, 0xffff, 0xffff, 0xffff, 0xf435, 0xffff, 0xffff, 0xffff, 0xffff, 0xf435, 0xf435, 0xf435, 0xffff, 0xf435, 0xf435,
            0xf435, 0xffff, 0xffff, 0xffff, 0xffff, 0xf435, 0xf435, 0xffff, 0xffff, 0xf435, 0xf435, 0xf435, 0xffff, 0xffff, 0xffff, 0xf435,
            0xf435, 0xffff, 0xffff, 0xffff, 0xffff, 0xf435, 0xf435, 0xf435, 0xf435, 0xf435, 0xf435, 0xffff, 0xffff, 0xffff, 0xffff, 0xf435,
            0xf435, 0xf435, 0xffff, 0xffff, 0xf435, 0xf435, 0xffff, 0xffff, 0xffff, 0xf435, 0xf435, 0xffff, 0xffff, 0xffff, 0xffff, 0xf435,
            0xf435, 0xf435, 0xf435, 0xf435, 0xf435, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xf435, 0xf435, 0xffff, 0xffff, 0xf435, 0xf435,
            0xf435, 0xf435, 0xf435, 0xf435, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xf435, 0xf435, 0xf435, 0xf435, 0xf435, 0xf435,
            0x0fff, 0xf435, 0xf435, 0xf435, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xf435, 0xf435, 0xf435, 0xf435, 0x0fff,
            0x0fff, 0xf435, 0xf435, 0xf435, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xf435, 0xf435, 0xf435, 0xf435, 0x0fff,
            0x0fff, 0x0fff, 0xf435, 0xf435, 0xf435, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xf435, 0xf435, 0xf435, 0xf435, 0x0fff, 0x0fff,
            0x0fff, 0x0fff, 0x0fff, 0xf435, 0xf435, 0xf435, 0xffff, 0xffff, 0xffff, 0xf435, 0xf435, 0xf435, 0xf435, 0x0fff, 0x0fff, 0x0fff,
            0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0xf435, 0xf435, 0xf435, 0xf435, 0xf435, 0xf435, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff,
        };
        surface = SDL_CreateSurfaceFrom(16,16, SDL_PIXELFORMAT_RGBA64, pixels, 0);

        // The icon is attached to the window pointer
        SDL_SetWindowIcon((SDL_Window*)data.sdlWindow, surface);

        // ...and the surface containing the icon pixel data is no longer required.
        SDL_DestroySurface(surface);
        #endif

        if ((SDL_Window*)data.sdlWindow == nullptr) // Check for an error when creating a window
        {
            FatalError("SDL Window could not be created");
        }

        // Create OpenGL Context
        //m_glContext = {SDL_GL_CreateContext((SDL_Window*)m_sdlWindow)};
        data.glContext = (void*)SDL_GL_CreateContext((SDL_Window*)data.sdlWindow);

        if (data.glContext == nullptr) // Check for an error when creating the OpenGL Context
        {
            FatalError("SDL_GL context could not be created!");
        }

        #ifdef __EMSCRIPTEN__
            int major, minor;
            SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
            SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);

            Log("OpenGLES version loaded: " + std::to_string(major) + "." + std::to_string(minor));
        #else
        // Load OpenGL
        GLenum error = glewInit();
        if (error != GLEW_OK) {
            FatalError("Could not init GLEW");
            SDL_GL_DestroyContext((SDL_GLContext)data.glContext);
            SDL_DestroyWindow((SDL_Window*)data.sdlWindow);
            SDL_Quit();
            exit(-1);
        }

        // Display OpenGL version
        int major, minor;
        SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
        SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);

        Log((std::string("OpenGL version loaded: ") + std::to_string(major) + std::string(".") + std::to_string(minor)).c_str());
        #endif

        

        // before a new frame is drawn we need to clear the buffer
        // the clear color will be the new value of all of the pixels
        // in that buffer
        SetClearColor(glm::vec4(0.05f, 0.05f, 0.05f, 1.0f));

        // VSYNC 0 off 1 on
        SDL_GL_SetSwapInterval(0);

        // Enable alpha blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        return 0;
    }

    void Window::SetWindowName(std::string _windowName)
    {
        CanisData& data = GetCanisData();
        SDL_SetWindowTitle((SDL_Window*)data.sdlWindow,_windowName.c_str());
    }

    void Window::SwapBuffer()
    {
        CanisData& data = GetCanisData();
        // After we draw our sprite and models to a window buffer
        // We want to display the one we were drawing to and
        // get the old buffer to start drawing our next frame to
        SDL_GL_SwapWindow((SDL_Window*)data.sdlWindow);
    }
    
    void Window::CenterMouse()
    {
        CanisData& data = GetCanisData();
        SetMousePosition(data.screenWidth/2, data.screenHeight/2);
    }

    void Window::SetMousePosition(int _x, int _y)
    {
        CanisData& data = GetCanisData();
        SDL_WarpMouseInWindow((SDL_Window*)data.sdlWindow, _x, data.screenHeight - _y);
    }

    void Window::ClearColor()
    {
        CanisData& data = GetCanisData();
        glClearColor(data.clearColor.r, data.clearColor.g, data.clearColor.b, data.clearColor.a);
    }

    void Window::SetClearColor(glm::vec4 _color)
    {
        CanisData& data = GetCanisData();
        data.clearColor = _color;
    }

    void Window::MouseLock(bool _isLocked)
    {
        CanisData& data = GetCanisData();
        data.mouseLock = _isLocked;
        if (_isLocked)
        {
            //SDL_CaptureMouse(SDL_TRUE);
            //SDL_SetRelativeMouseMode(SDL_TRUE);
        }
        else
        {
            //SDL_CaptureMouse(SDL_FALSE);
            //SDL_SetRelativeMouseMode(SDL_FALSE);
        }
    }

    void Window::ToggleFullScreen()
    {
        CanisData& data = GetCanisData();
        data.fullscreen = !data.fullscreen;

        SDL_SetWindowFullscreen((SDL_Window*)data.sdlWindow, data.fullscreen);
    }

    bool Window::GetVSync()
    {
        return true;//(bool)SDL_GL_GetSwapInterval(0);
    }

    void Window::SetVSync(bool _vsync)
    {
        SDL_GL_SetSwapInterval((int)_vsync);
    }

    void Window::SetWindowSize(int _width, int _height)
    {
        CanisData& data = GetCanisData();
        if ( _width == data.screenWidth && _height == data.screenHeight)
            return;
        
        data.resized = true;

        data.screenWidth = _width;
        data.screenHeight = _height;

        std::string message = std::string("width: ") + std::to_string(data.screenWidth) + std::string(" height: ") + std::to_string(data.screenHeight);

        Log(message.c_str());
    }

    void Window::SetResized(bool _resized)
    {
        GetCanisData().resized = _resized;
    }

    bool Window::IsResized()
    {
        return GetCanisData().resized;
    }
} // end of Canis namespace

namespace CSharpLayer
{
    void CSharpLayer_SetTitle(const char *_title)
    {
        Canis::Window::SetWindowName(std::string(_title));
    }

    void CSharpLayer_SetWindowSize(int _width, int _height)
    {
        Canis::Window::SetWindowSize(_width, _height);
    }
}
