#pragma once
#include <Canis/Math.hpp>

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

        void Clear() const;
        void SetClearColor(Color _color);
        Color GetClearColor() { return m_clearColor; }
        void SwapBuffer() const;

        void SetWindowSize(int _width, int _height);
        void SetResized(bool _resized);
        bool IsResized();

        void* GetSDLWindow() { return m_window; }
        void* GetGLContext() { return m_context; }

        Sync GetSync();
        void SetSync(Sync _type);

    private:
        void* m_window = nullptr;
        void* m_context = nullptr;

        Color m_clearColor;

        bool m_shouldClose = false;
        bool m_resized = false;

        int m_screenWidth = 0;
        int m_screenHeight = 0;

        void InitGL();
    };

} // namespace Canis
