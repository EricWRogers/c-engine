#pragma once
#include <string>
#include <glm/glm.hpp>
#include <vector>

#include "Debug.hpp"

namespace Canis
{
    enum WindowFlags
    {
        FULLSCREEN = 1,
        BORDERLESS = 2,
        RESIZEABLE = 4
    };

    namespace Window
    {
        int Create(std::string _windowName, int _screenWidth, int _screenHeight, unsigned int _currentFlags);
        void SetWindowName(std::string _windowName);

        void SwapBuffer();
        
        void CenterMouse();
        void SetMousePosition(int _x, int _y);

        void ClearColor();
        void SetClearColor(glm::vec4 _color);
        static glm::vec4 GetScreenColor() {
            CanisData& data = GetCanisData();
            return data.clearColor;
        }

        void MouseLock(bool _isLocked);
        static bool GetMouseLock() {
            CanisData& data = GetCanisData();
            return data.mouseLock;
        }

        static int GetScreenWidth() {
            CanisData& data = GetCanisData();
            return data.screenWidth;
        }
        static int GetScreenHeight() {
            CanisData& data = GetCanisData();
            return data.screenHeight;
        }

        void ToggleFullScreen();
        void SetWindowSize(int _width, int _height);
        void SetResized(bool _resized);
        bool IsResized();

        bool GetVSync();
        void SetVSync(bool _vsync);
    }
} // end of Canis namespace

namespace CSharpLayer
{
    extern "C" {
        void CSharpLayer_SetTitle(const char* _title);
        void CSharpLayer_SetWindowSize(int _width, int _height);
        void CSharpLayer_SetBackgroundColor(float _red, float _green, float _blue, float _alpha);
    }
}
