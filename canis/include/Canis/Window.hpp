#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_opengles2.h>

namespace Canis
{

    class Window
    {
    public:
        typedef enum Sync
        {
            IMMEDIATE = 0,
            VSYNC = 1,
            ADAPTIVE = -1
        } Sync;

        Window(const char *title, int width, int height);
        ~Window();

        int GetScreenWidth() { return m_screenWidth; }
        int GetScreenHeight() { return m_screenHeight; }

        void Clear(float r, float g, float b, float a) const;
        void SwapBuffer() const;

        void SetWindowSize(int _width, int _height);
        void SetResized(bool _resized);
        bool IsResized();

        void *GetSDLWindow() { return m_window; }

        Sync GetSync();
        void SetSync(Sync _type);

    private:
        SDL_Window *m_window = nullptr;
        SDL_GLContext m_context = nullptr;

        bool m_shouldClose = false;
        bool m_resized = false;

        int m_screenWidth = 0;
        int m_screenHeight = 0;

        void InitGL();
    };

} // namespace Canis
