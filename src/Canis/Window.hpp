#pragma once
#include <string>

#include "Debug.hpp"

namespace Canis
{
    enum WindowFlags
    {
        FULLSCREEN = 1,
        BORDERLESS = 16
    };

    class Window
    {
    public:
        Window();
        ~Window();

        int CreateFullScreen(std::string _windowName);
        int Create(std::string _windowName, int _screenWidth, int _screenHeight, unsigned int _currentFlags);
        void SetWindowName(std::string _windowName);

        void SwapBuffer();
        void MouseLock(bool _isLocked);
        bool GetMouseLock() { return mouseLock; }

        int GetScreenWidth() { return screenWidth; }
        int GetScreenHeight() { return screenHeight; }

        void* GetSDLWindow() { return sdlWindow; }
        void* GetGLContext() { return m_glContext; }

        void ToggleFullScreen();

        float fps;

    private:
        void *sdlWindow;
        void *m_glContext;
        int screenWidth, screenHeight;
        bool m_fullscreen = false;
        bool mouseLock = false;
    };
} // end of Canis namespace